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


#include "geomtools.h"
#include "io.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Triangulation_vertex_base_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Polygon_2.h>
#include <iostream>

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
typedef CGAL::Triangulation_vertex_base_2<Gt>				Vb;
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, Gt>	Fbb;
typedef CGAL::Constrained_triangulation_face_base_2<Gt, Fbb>		Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb>				Tds;
typedef CGAL::Exact_predicates_tag									Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<Gt, Tds, Itag>	CDT;
typedef CDT::Point													Point;
typedef CGAL::Polygon_2<Gt>											Polygon_2;

bool triangle_contains_segment(Triangle t, std::string a, std::string b) {
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
	std::vector<Point3> &vertices,
	std::unordered_map< std::string, std::vector<Point3>::size_type > &vertices_map,
	std::vector<Triangle> &triangles,
	const std::vector<Point3> &lidarpts) {
	CDT cdt;

	Ring2 oring = bg::exterior_ring(*pgn);
	auto irings = bg::interior_rings(*pgn);

	Polygon_2 poly;
	int ringi = 0;
	//-- add the outer ring as a constraint
	for (int i = 0; i < oring.size(); i++) {
		poly.push_back(Point(bg::get<0>(oring[i]), bg::get<1>(oring[i]), (float(z[ringi][i]) / 100.0)));
		//points.push_back(Point(bg::get<0>(oring[i]), bg::get<1>(oring[i])));
	}
	cdt.insert_constraint(poly.vertices_begin(), poly.vertices_end(), true);
	poly.clear();
	ringi++;

	//-- add the inner ring(s) as a constraint
	if (irings.size() > 0) {
		for (auto iring : irings) {
			for (int i = 0; i < iring.size(); i++) {
				poly.push_back(Point(bg::get<0>(iring[i]), bg::get<1>(iring[i]), (float(z[ringi][i]) / 100.0)));
			}
			cdt.insert_constraint(poly.vertices_begin(), poly.vertices_end(), true);
			poly.clear();
			ringi++;
		}
	}

	//-- add the lidar points to the CDT, if any
	for (auto &pt : lidarpts) {
		cdt.insert(Point(bg::get<0>(pt), bg::get<1>(pt), bg::get<2>(pt)));
	}

	//Mark facets that are inside the domain bounded by the polygon
	mark_domains(cdt);

	unsigned index = 0;
	//int count = 0;

	if (!cdt.is_valid()) {
		std::clog << "CDT is invalid." << std::endl;
	}

  // Create a unique list of vertices in this CDT and create the triangle
  for (CDT::Finite_faces_iterator fit = cdt.finite_faces_begin();
    fit != cdt.finite_faces_end(); ++fit) {
    if (fit->info().in_domain()) {
      Triangle t;
      Point3 p;
      std::string key;
      p = Point3(fit->vertex(0)->point().x(), fit->vertex(0)->point().y(), fit->vertex(0)->point().z());
      key = gen_key_bucket(&p);
      if (vertices_map.find(key) == vertices_map.end()) {
        vertices.push_back(p);
        vertices_map[key] = vertices.size() - 1;
      }
      t.v0 = key;

      p = Point3(fit->vertex(1)->point().x(), fit->vertex(1)->point().y(), fit->vertex(1)->point().z());
      key = gen_key_bucket(&p);
      if (vertices_map.find(key) == vertices_map.end()) {
        vertices.push_back(p);
        vertices_map[key] = vertices.size() - 1;
      }
      t.v1 = key;

      p = Point3(fit->vertex(2)->point().x(), fit->vertex(2)->point().y(), fit->vertex(2)->point().z());
      key = gen_key_bucket(&p);
      if (vertices_map.find(key) == vertices_map.end()) {
        vertices.push_back(p);
        vertices_map[key] = vertices.size() - 1;
      }
      t.v2 = key;

      if (t.v0 != t.v1 && t.v0 != t.v2 && t.v1 != t.v2) {
        triangles.push_back(t);
      }
    }
  }

	return true;
}