/*
  3dfier: takes 2D GIS datasets and "3dfies" to create 3D city models.

  Copyright (C) 2015-2016  3D geoinformation research group, TU Delft

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

#include "io.h"
#include "geomtools.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Triangulation_vertex_base_with_id_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Polygon_2.h>

#include <vector>
#include <unordered_set>
#include <iterator>
#include <memory>
#include <boost/heap/binomial_heap.hpp>

// binomial heap for greedy insertion code
struct PointXYHash
{
  std::size_t operator()(Point3 const& p) const noexcept
  {
    std::size_t h1 = std::hash<double>{}(bg::get<0>(p));
    std::size_t h2 = std::hash<double>{}(bg::get<1>(p));
    return h1 ^ (h2 << 1);
  }
};
struct PointXYEqual
{
  std::size_t operator()(Point3 const& p1, Point3 const& p2) const noexcept
  {
    auto ex = bg::get<0>(p1) == bg::get<0>(p2);
    auto ey = bg::get<1>(p1) == bg::get<1>(p2);
    return ex && ey;
  }
};
struct point_error {
  point_error(int i, double e) : index(i), error(e){}
  int index;
  double error;
};
struct compare_error {
  inline bool operator()
    (const point_error &e1 , const point_error &e2) const {
      return e1.error < e2.error;
}
};
typedef boost::heap::binomial_heap<point_error, boost::heap::compare<compare_error>> Heap;
typedef Heap::handle_type heap_handle;

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
  std::vector<heap_handle>* points_inside = nullptr;
  CGAL::Plane_3<K>* plane = nullptr;
};
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, Gt>	Fbb;
typedef CGAL::Constrained_triangulation_face_base_2<Gt, Fbb>		Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb>				Tds;
typedef CGAL::Exact_predicates_tag									Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<Gt, Tds, Itag>	CDT;
typedef CDT::Point													Point;
typedef CGAL::Polygon_2<Gt>											Polygon_2;

inline double compute_error(Point &p, CDT::Face_handle &face);
void greedy_insert(CDT &T, const std::vector<Point3> &pts, double threshold);

bool triangle_contains_segment(Triangle t, int a, int b) {
  if ((t.v0 == a) && (t.v1 == b))
    return true;
  if ((t.v1 == a) && (t.v2 == b))
    return true;
  if ((t.v2 == a) && (t.v0 == b))
    return true;
  return false;
}

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

bool getCDT(const Polygon2* pgn,
  const std::vector< std::vector<int> > &z,
  std::vector< std::pair<Point3, std::string> > &vertices,
  std::vector<Triangle> &triangles,
  const std::vector<Point3> &lidarpts,
  double tinsimp_threshold) {
  CDT cdt;

  //-- gather all rings
  std::vector<Ring2> rings;
  rings.push_back(bg::exterior_ring(*(pgn)));
  for (auto& iring : bg::interior_rings(*(pgn)))
    rings.push_back(iring);

  Polygon_2 poly;
  int ringi = -1;
  for (auto ring : rings) {
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
    std::clog << "CDT is invalid.\n";
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

std::string gen_key_bucket(Point2* p) {
  char* buf = new char[50];
  std::sprintf(buf, "%.3f %.3f", p->get<0>(), p->get<1>());
  return buf;
}

std::string gen_key_bucket(Point3* p) {
  char* buf = new char[50];
  std::sprintf(buf, "%.3f %.3f %.3f", p->get<0>(), p->get<1>(), p->get<2>());
  return buf;
}

std::string gen_key_bucket(Point3* p, int z) {
  char* buf = new char[50];
  std::sprintf(buf, "%.3f %.3f %d", p->get<0>(), p->get<1>(), z);
  return buf;
}

double distance(const Point2 &p1, const Point2 &p2) {
  return sqrt((p1.x() - p2.x())*(p1.x() - p2.x()) + (p1.y() - p2.y())*(p1.y() - p2.y()));
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
    face->info().points_inside = new std::vector<heap_handle>();

  auto plane = face->info().plane;
  auto interpolate = - plane->a()/plane->c() * p.x() - plane->b()/plane->c()*p.y() - plane->d()/plane->c();
  double error = std::fabs(interpolate - p.z());
  return error;
}

void greedy_insert(CDT &T, const std::vector<Point3> &pts, double threshold) {
  // assumes all lidar points are inside a triangle
  Heap heap;

  // compute initial point errors, build heap, store point indices in triangles
  {
    std::unordered_set<Point3, PointXYHash, PointXYEqual> set;
    for(int i=0; i<pts.size(); i++){
      auto p3 = pts[i];
      // detect and skip duplicate points
      auto not_duplicate = set.insert(p3).second;
      if(not_duplicate){
        auto p = Point(bg::get<0>(p3), bg::get<1>(p3), bg::get<2>(p3));
        auto face = T.locate(p);
        auto e = compute_error(p, face);
        auto handle = heap.push(point_error(i,e));
        face->info().points_inside->push_back(handle);
      }
    }
  }
  // insert points, update errors of affected triangles until threshold error is reached
  double error;
  do{
    if (heap.empty())
      break;
    
    auto maxelement = heap.top();
    error = maxelement.error;
    auto max_element_index = maxelement.index;
    auto p3 = pts[maxelement.index];
    auto max_p = Point(bg::get<0>(p3), bg::get<1>(p3), bg::get<2>(p3));

    std::vector<CDT::Face_handle> faces;
    T.get_conflicts ( max_p, std::back_inserter(faces) );

    auto face_hint = faces[0];
    auto v = T.insert(max_p, face_hint);
    face_hint = v->face();
    
    std::vector<heap_handle> points_to_update;
    for (auto face : faces) {
      if (face->info().plane){
        delete face->info().plane;
        face->info().plane = nullptr;
      }
      if (face->info().points_inside) {
        for (auto h :*face->info().points_inside){
          if(max_element_index != (*h).index)
            points_to_update.push_back(h);
        }
        face->info().points_inside->clear();
      }
    }
    heap.pop();
    
    for (auto curelement : points_to_update){
      auto p3 = pts[(*curelement).index];
      auto p = Point(bg::get<0>(p3), bg::get<1>(p3), bg::get<2>(p3));
      auto containing_face = T.locate(p, face_hint);
      const double e = compute_error(p, containing_face);
      heap.update(curelement, point_error((*curelement).index,e));
      containing_face->info().points_inside->push_back(curelement);
    }
  }while(error > threshold);

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