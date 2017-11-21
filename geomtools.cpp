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
#include <iostream>

#include <vector>

struct FaceInfo2
{
  FaceInfo2() {}
  int nesting_level;
  bool in_domain() {
    return nesting_level % 2 == 1;
  }
};

typedef CGAL::Exact_predicates_inexact_constructions_kernel			K;
typedef CGAL::Projection_traits_xy_3<K>								Gt;
typedef CGAL::Triangulation_vertex_base_with_id_2<Gt>				Vb;
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, Gt>	Fbb;
typedef CGAL::Constrained_triangulation_face_base_2<Gt, Fbb>		Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb>				Tds;
typedef CGAL::Exact_predicates_tag									Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<Gt, Tds, Itag>	CDT;
typedef CDT::Point													Point;
typedef CGAL::Polygon_2<Gt>											Polygon_2;

typedef std::map<CDT::Vertex_handle, double> Vertex_map; // a vertex and an error
double estimateZ_LIN(CDT &dt, CDT::Vertex_handle);
void updateMap(CDT &dt, Vertex_map &vmap, CDT::Vertex_handle v);  
void simplify(CDT &dt, double threshold);


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

  Ring2 oring = bg::exterior_ring(*pgn);
  auto irings = bg::interior_rings(*pgn);

  Polygon_2 poly;
  int ringi = 0;
  //-- add the outer ring as a constraint
  for (int i = 0; i < oring.size(); i++) {
    poly.push_back(Point(bg::get<0>(oring[i]), bg::get<1>(oring[i]), z_to_float(z[ringi][i])));
    //points.push_back(Point(bg::get<0>(oring[i]), bg::get<1>(oring[i])));
  }
  cdt.insert_constraint(poly.vertices_begin(), poly.vertices_end(), true);
  poly.clear();
  ringi++;

  //-- add the inner ring(s) as a constraint
  if (irings.size() > 0) {
    for (auto iring : irings) {
      for (int i = 0; i < iring.size(); i++) {
        poly.push_back(Point(bg::get<0>(iring[i]), bg::get<1>(iring[i]), z_to_float(z[ringi][i])));
      }
      cdt.insert_constraint(poly.vertices_begin(), poly.vertices_end(), true);
      poly.clear();
      ringi++;
    }
  }

  //-- add the lidar points to the CDT, if any
  if (lidarpts.size() > 0) {
    for (auto &pt : lidarpts) {
      cdt.insert(Point(bg::get<0>(pt), bg::get<1>(pt), bg::get<2>(pt)));
    } 
    //-- simplify lidar points
    if (tinsimp_threshold != 0)
      simplify(cdt, tinsimp_threshold);
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


//--- TIN Simplification

// estimate the depth on (x,y) position of vertex v after removal of that vertex, using Linear interpolation.
double estimateZ_LIN(CDT &dt, CDT::Vertex_handle v)
{
  Point p1,p2,p3,q = v->point();
  
  CDT t2;
  CDT::Vertex_circulator vc = dt.incident_vertices(v), done(vc);
  do{
    if(!dt.is_infinite(vc))
      t2.insert(vc->point());
  } while(++vc!=done);
  
  CDT::Face_handle face = t2.locate(q);

  p1 = face->vertex(0)->point();
  p2 = face->vertex(1)->point();
  p3 = face->vertex(2)->point();
  CGAL::Plane_3<K> plane(p1,p2,p3);
  
  return - plane.a()/plane.c() * q.x() - plane.b()/plane.c()*q.y() - plane.d()/plane.c();
}

void updateMap(CDT &dt, Vertex_map &vmap, CDT::Vertex_handle v)
{
  // check if vertex v is not in a constrained edge or on the convex hull
  CDT::Face_circulator iFace = dt.incident_faces(v), done(iFace);
  do{
    int vi = iFace->index(v);
    if(iFace->is_constrained(CDT::cw(vi)) || iFace->is_constrained(CDT::ccw(vi))) 
      return;
    if(iFace->has_vertex(dt.infinite_vertex()))
      return;
  } while(++iFace != done);

  // Compute and store vertical error
  double e = v->point().z() - estimateZ_LIN(dt, v);
  vmap[v] = std::fabs(e);
}

// TIN simplification algorithm based on article "A drop heuristic conversion method for extracting irregular network for digital elevation models" by J Lee (1989)
void simplify(CDT &dt, double threshold)
{
  Vertex_map vmap;
  double min_error = threshold-1;
  CDT::Vertex_handle min_error_vertex;
   
  // compute initial errors for all vertices
  for( CDT::Finite_vertices_iterator vit=dt.finite_vertices_begin() ; vit != dt.finite_vertices_end(); ++vit ) {
      updateMap(dt, vmap, vit);
  }
  
  // drop vertices from triangulation until error threshold is violated
  int numberOfDroppedPoints = 0;
  while(true) {
    //find vertex with smallest error
    Vertex_map::iterator currentPosIt = vmap.begin();
    min_error = threshold;
    for (Vertex_map::iterator it = vmap.begin(); it != vmap.end(); ++it) {
        if (min_error >= it->second) {
          min_error = it->second;
          min_error_vertex = it->first;
          currentPosIt = it;
        }
    }
    
    //stop in case the threshold is reached
    if (min_error == threshold)
      break;

    //store its neighbours
    CDT::Vertex_circulator iVertex = dt.incident_vertices(min_error_vertex), done(iVertex);
    std::vector<CDT::Vertex_handle> neighbours;
    do{
      if(!dt.is_infinite(iVertex))
        neighbours.push_back(iVertex);
    } while(++iVertex != done);
    
    //remove vertex
    dt.remove(min_error_vertex);
    vmap.erase(min_error_vertex);
    numberOfDroppedPoints++;
    
    //update vmap for neighbours
    for(std::vector<CDT::Vertex_handle>::iterator iVertex = neighbours.begin(); iVertex != neighbours.end(); ++iVertex){
      updateMap(dt, vmap, *iVertex);
    }

    //stop in case there are no more vertices left
    if(vmap.size()==0)
      break;
  }
}
