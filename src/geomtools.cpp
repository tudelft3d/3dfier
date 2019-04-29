/*
  3dfier: takes 2D GIS datasets and "3dfies" to create 3D city models.

  Copyright (C) 2015-2019  3D geoinformation research group, TU Delft

  This file is part of 3dfier.

  3dfier is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  3dfier is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with 3difer.  If not, see <http://www.gnu.org/licenses/>.

  For any information or further details about the use of 3dfier, contact
  Hugo Ledoux
  <h.ledoux@tudelft.nl>
  Faculty of Architecture & the Built Environment
  Delft University of Technology
  Julianalaan 134, Delft 2628BL, the Netherlands
*/

#include "geomtools.h"
#include "io.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Triangulation_vertex_base_with_id_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Polygon_2.h>

#include <vector>
#include <unordered_set>
#include <boost/heap/fibonacci_heap.hpp>

// fibonacci heap for greedy insertion code
struct point_error {
  point_error(int i, double e) : index(i), error(e){}
  int index;
  double error;
  
  bool operator<(point_error const & rhs) const
  {
    return error < rhs.error;
  }
};
typedef boost::heap::fibonacci_heap<point_error> Heap;
typedef Heap::handle_type heap_handle;
typedef std::vector<heap_handle> heap_handle_vec;

typedef CGAL::Exact_predicates_inexact_constructions_kernel			K;
typedef CGAL::Projection_traits_xy_3<K>								Gt;
typedef CGAL::Triangulation_vertex_base_with_id_2<Gt>				Vb;
struct FaceInfo2
{
  FaceInfo2() {}
  int nesting_level;
  bool in_domain() {
    return nesting_level % 2 == 1;
  }
  heap_handle_vec* points_inside = nullptr;
  CGAL::Plane_3<K>* plane = nullptr;
};
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, Gt>  Fbb;
typedef CGAL::Constrained_triangulation_face_base_2<Gt, Fbb>      Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb>              Tds;
typedef CGAL::Exact_predicates_tag                                Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<Gt, Tds, Itag> CDT;
typedef CDT::Point                                                Point;
typedef CGAL::Polygon_2<Gt>                                       Polygon_2;

struct PointXYHash {
  std::size_t operator()(Point const& p) const noexcept {
    std::size_t h1 = std::hash<double>{}(p.x());
    std::size_t h2 = std::hash<double>{}(p.y());
    return h1 ^ (h2 << 1);
  }
};
struct PointXYEqual {
  bool operator()(Point const& p1, Point const& p2) const noexcept {
    return CGAL::compare_xy(p1, p2) == CGAL::Comparison_result::EQUAL;

    //auto ex = p1.x() == p2.x();
    //auto ey = p1.y() == p2.y();
    //return ex && ey;
  }
};

inline double compute_error(Point &p, CDT::Face_handle &face);
void greedy_insert(CDT &T, const std::vector<Point3> &pts, double threshold);

void mark_domains(CDT& ct,
  CDT::Face_handle start,
  int index,
  std::list<CDT::Edge>& border) {
  if (start->info().nesting_level != -1) {
    return;
  }
  std::list<CDT::Face_handle> queue;
  queue.push_back(start);
  while (!queue.empty()) {
    CDT::Face_handle fh = queue.front();
    queue.pop_front();
    if (fh->info().nesting_level == -1) {
      fh->info().nesting_level = index;
      for (int i = 0; i < 3; i++) {
        CDT::Edge e(fh, i);
        CDT::Face_handle n = fh->neighbor(i);
        if (n->info().nesting_level == -1) {
          if (ct.is_constrained(e)) border.push_back(e);
          else queue.push_back(n);
        }
      }
    }
  }
}

//explore set of facets connected with non constrained edges,
//and attribute to each such set a nesting level.
//We start from facets incident to the infinite vertex, with a nesting
//level of 0. Then we recursively consider the non-explored facets incident 
//to constrained edges bounding the former set and increase the nesting level by 1.
//Facets in the domain are those with an odd nesting level.
void mark_domains(CDT& cdt) {
  for (CDT::All_faces_iterator it = cdt.all_faces_begin(); it != cdt.all_faces_end(); ++it) {
    it->info().nesting_level = -1;
  }
  std::list<CDT::Edge> border;
  mark_domains(cdt, cdt.infinite_face(), 0, border);
  while (!border.empty()) {
    CDT::Edge e = border.front();
    border.pop_front();
    CDT::Face_handle n = e.first->neighbor(e.second);
    if (n->info().nesting_level == -1) {
      mark_domains(cdt, n, e.first->info().nesting_level + 1, border);
    }
  }
}

bool getCDT(Polygon2* pgn,
  const std::vector< std::vector<int> > &z,
  std::vector< std::pair<Point3, std::string> > &vertices,
  std::vector<Triangle> &triangles,
  const std::vector<Point3> &lidarpts,
  double tinsimp_threshold) {
  CDT cdt;

  //-- gather all rings
  std::vector<Ring2> rings;
  rings.push_back(pgn->outer());
  for (Ring2& iring : pgn->inners())
    rings.push_back(iring);

  Polygon_2 poly;
  int ringi = -1;
  for (Ring2& ring : rings) {
    ringi++;
    for (int i = 0; i < ring.size(); i++) {
      poly.push_back(Point(bg::get<0>(ring[i]), bg::get<1>(ring[i]), z_to_float(z[ringi][i])));
    }
    cdt.insert_constraint(poly.vertices_begin(), poly.vertices_end(), true);
    poly.clear();
  }

  //-- add the lidar points to the CDT, if any
  if (lidarpts.size() > 0) {
    if (tinsimp_threshold != 0)
      greedy_insert(cdt, lidarpts, tinsimp_threshold);
    else {
      for (auto &pt : lidarpts) {
        cdt.insert(Point(bg::get<0>(pt), bg::get<1>(pt), bg::get<2>(pt)));
      }
    }
  }

  //Mark facets that are inside the domain bounded by the polygon
  mark_domains(cdt);

  unsigned index = 0;
  int count = 0;

  if (!cdt.is_valid()) {
    throw std::runtime_error("CDT is invalid.");
  }
  for (CDT::Finite_vertices_iterator vit = cdt.finite_vertices_begin();
    vit != cdt.finite_vertices_end(); ++vit) {
    Point3 p = Point3(vit->point().x(), vit->point().y(), vit->point().z());
    vertices.push_back(std::make_pair(p, gen_key_bucket(&p)));
    vit->id() = index++;
  }

  for (CDT::Finite_faces_iterator fit = cdt.finite_faces_begin();
    fit != cdt.finite_faces_end(); ++fit) {
    if (fit->info().in_domain()) {
      Triangle t;
      t.v0 = fit->vertex(0)->id();
      t.v1 = fit->vertex(1)->id();
      t.v2 = fit->vertex(2)->id();
      triangles.push_back(t);
      count++;
    }
  }
  return true;
}

std::string gen_key_bucket(const Point2* p) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(3) << p->get<0>() << " " << p->get<1>();
  return ss.str();
}

std::string gen_key_bucket(const Point3* p) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(3) << p->get<0>() << " " << p->get<1>() << " " << std::setprecision(2) << p->get<2>();
  return ss.str();
}

std::string gen_key_bucket(const Point3* p, float z) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(3) << p->get<0>() << " " << p->get<1>() << " " << std::setprecision(2) << z;
  return ss.str();
}

double distance(const Point2 &p1, const Point2 &p2) {
  double dx = p1.x() - p2.x();
  double dy = p1.y() - p2.y();
  return sqrt(dx * dx + dy * dy);
}

double sqr_distance(const Point2 &p1, const Point2 &p2) {
  double dx = p1.x() - p2.x();
  double dy = p1.y() - p2.y();
  return dx * dx + dy * dy;
}

//--- TIN Simplification
// Greedy insertion/incremental refinement algorithm adapted from "Fast polygonal approximation of terrain and height fields" by Garland, Michael and Heckbert, Paul S.
inline double compute_error(Point &p, CDT::Face_handle &face) {
  if(!face->info().plane)
    face->info().plane = new CGAL::Plane_3<K>(
      face->vertex(0)->point(),
      face->vertex(1)->point(),
      face->vertex(2)->point());
  if(!face->info().points_inside)
    face->info().points_inside = new heap_handle_vec();

  auto plane = face->info().plane;
  auto interpolate = - plane->a()/plane->c() * p.x() - plane->b()/plane->c()*p.y() - plane->d()/plane->c();
  double error = std::fabs(interpolate - p.z());
  return error;
}

void greedy_insert(CDT &T, const std::vector<Point3> &pts, double threshold) {
  // assumes all lidar points are inside a triangle
  Heap heap;

  // Convert all elevation points to CGAL points
  std::unordered_set<Point, PointXYHash, PointXYEqual> cpts_set;
  std::vector<Point> cpts;
  cpts.reserve(pts.size());
  for (auto& p : pts) {
    auto r = cpts_set.insert(Point(bg::get<0>(p), bg::get<1>(p), bg::get<2>(p)));
    if (!r.second) {
      std::cout << "Duplicate point: " << *(r.first) << " && " << Point(bg::get<0>(p), bg::get<1>(p), bg::get<2>(p)) << std::endl;
    }
  }
  cpts.assign(cpts_set.begin(), cpts_set.end());
  std::unordered_set<Point, PointXYHash, PointXYEqual>().swap(cpts_set);

  // compute initial point errors, build heap, store point indices in triangles
  {
    for (int i = 0; i < cpts.size(); i++) {
      auto p = cpts.at(i);
      CDT::Locate_type lt;
      int li;
      CDT::Face_handle face = T.locate(p, lt, li);
      if (lt == CDT::FACE) {
        auto e = compute_error(p, face);
        auto handle = heap.push(point_error(i, e));
        face->info().points_inside->push_back(handle);
      }
      else {
        std::cout << "CDT insert; point location not in face but ";
        if (lt == CDT::VERTEX) {
          std::cout << "on vertex.";
        }
        else if (lt == CDT::EDGE) {
          std::cout << "on edge.";
        }
        else if (lt == CDT::OUTSIDE_CONVEX_HULL) {
          std::cout << "outside convex hull.";
        }
        else if (lt == CDT::OUTSIDE_AFFINE_HULL) {
          std::cout << "outside affine hull.";
        }
        std::cout << " Point; " << std::fixed << std::setprecision(3) << p << std::endl;
      }
    }
  }
  
  // insert points, update errors of affected triangles until threshold error is reached
  while (!heap.empty() && heap.top().error > threshold){
    // get top element (with largest error) from heap
    auto maxelementindex = heap.top().index;
    auto max_p = cpts.at(maxelementindex);

    // get triangles that will change after inserting this max_p
    std::vector<CDT::Face_handle> faces;
    T.get_conflicts(max_p, std::back_inserter(faces));

    // handle case where max_p somehow coincides with a polygon vertex
    if (faces.size()==0) {
        // this should return the already existing vertex
        auto v = T.insert(max_p);

        // check the incident faces and erase any references to max_p
        CDT::Face_circulator fcirculator = T.incident_faces(v), done(fcirculator);
        auto start = fcirculator;
        do {
          auto face = *fcirculator;
          if (face.info().points_inside) {
            // collect the heap_handles that need to be removed
            std::vector<heap_handle_vec::iterator> to_erase;
            for (auto it = face.info().points_inside->begin(); it != face.info().points_inside->end(); ++it) {
              if (maxelementindex == (**it).index) {
                to_erase.push_back(it);
              }
            }
            // remove the collected heap_handles
            for (auto it : to_erase) {
              face.info().points_inside->erase(it);
            }
          }
        } while (++fcirculator != done);
        
        // remove this points from the heap
        heap.pop();
        
        // skip to next iteration
        continue;
    }

    // update clear info of triangles that just changed, collect points that were inside these triangles
    heap_handle_vec points_to_update;

    bool maxe = false;
    int pointless = 0;
    for (auto face : faces) {
      if (face->info().plane) {
        delete face->info().plane;
        face->info().plane = nullptr;
      }
      if (face->info().points_inside) {
        for (auto h : *face->info().points_inside) {
          if (maxelementindex != (*h).index) {
            points_to_update.push_back(h);


            //TESTING
            //std::cout << "pushed: " << (*h).index << std::endl;

            //int index = (*h).index;
            //if (index < 0 || index >= cpts.size()) {
            //  std::cout << "looking for index: " << maxelementindex << std::endl;
            //  std::cout << "index out of range: " << index << std::endl;
            //}
          }
          //else {
          //  maxe = true;
          //}
        }
        heap_handle_vec().swap((*face->info().points_inside));
      }
      //else {
      //  pointless++;
      //}
    }
    //std::cout << "Pointsless faces: " << pointless << std::endl;
    //if (!maxe) {
    //  std::cout << "maxelement " << maxelementindex << " not found";

    //  // Locate the face where
    //  CDT::Locate_type lt;
    //  int li;
    //  CDT::Face_handle containing_face = T.locate(cpts.at(maxelementindex), lt, li, faces[0]);
    //  std::cout << "; point location: ";
    //  if (lt == CDT::EDGE) {
    //    std::cout << "on edge.";
    //  }
    //  else if (lt == CDT::FACE) {
    //    std::cout << "in face.";
    //  }
    //  else if (lt == CDT::VERTEX) {
    //    std::cout << "on vertex.";
    //  }
    //  else if (lt == CDT::OUTSIDE_CONVEX_HULL) {
    //    std::cout << "outside convex hull.";
    //  }
    //  else if (lt == CDT::OUTSIDE_AFFINE_HULL) {
    //    std::cout << "outside affine hull.";
    //  }
    //  std::cout << " Point; " << std::fixed << std::setprecision(3) << cpts.at(maxelementindex) << std::endl;
    //  std::cout << " Vertex; " << std::fixed << std::setprecision(3) << containing_face->vertex(li)->point() << std::endl;

    //  heap_handle_vec* pi = containing_face->info().points_inside;
    //  // Delete stray handle
    //  for (auto h = pi->begin(); h != pi->end(); h++) {
    //    //for (auto h : pi) {
    //    if (maxelementindex == (**h).index) {
    //      containing_face->info().points_inside->erase(h);
    //      break;
    //    }
    //  }
    //}

    // insert max_p in triangulation
    auto face_hint = faces[0];
    auto v = T.insert(max_p, face_hint);
    face_hint = v->face();

    // remove this points from the heap
    heap.pop();

    // update the errors of affected elevation points
    for (auto curelement : points_to_update){
      int index = (*curelement).index;
      //std::cout << "pulled: " << (*curelement).index << std::endl;
      //if (index > cpts.size()) {
      //  std::cout << "index out of range: " << index << std::endl;
      //}
      auto p = cpts.at(index);
      //auto containing_face = T.locate(p, face_hint);
      CDT::Locate_type lt;
      int li;
      CDT::Face_handle containing_face = T.locate(p, lt, li, face_hint);
      if (lt == CDT::EDGE || lt == CDT::FACE) {
        const double e = compute_error(p, containing_face);
        const point_error new_pe = point_error(index, e);
        heap.update(curelement, new_pe);
        containing_face->info().points_inside->push_back(curelement);
      }
      else {
        std::cout << "CDT update; point location not in face but ";
        if (lt == CDT::VERTEX) {
          std::cout << "on vertex.";
        }
        else if (lt == CDT::OUTSIDE_CONVEX_HULL) {
          std::cout << "outside convex hull.";
        }
        else if (lt == CDT::OUTSIDE_AFFINE_HULL) {
          std::cout << "outside affine hull.";
        }
        std::cout << " Point (index " << index << "); " << std::fixed << std::setprecision(3) << p << std::endl;
      }
    }
  }

  //cleanup the stuff I put in face info of triangles
  for (CDT::Finite_faces_iterator fit = T.finite_faces_begin();
    fit != T.finite_faces_end(); ++fit) {
      if (fit->info().plane){
        delete fit->info().plane;
        fit->info().plane = nullptr;
      }
      if (fit->info().points_inside) {
        delete fit->info().points_inside;
        fit->info().points_inside = nullptr;
      }
    }

}
