/*
 Copyright (c) 2015 Hugo Ledoux

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#include "geomtools.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Triangulation_vertex_base_with_info_2.h>
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
typedef CGAL::Triangulation_vertex_base_with_info_2<std::set<Point3>::iterator, Gt>				Vb;
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, Gt>	Fbb;
typedef CGAL::Constrained_triangulation_face_base_2<Gt, Fbb>		Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb>				Tds;
typedef CGAL::Exact_predicates_tag									Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<Gt, Tds, Itag>	CDT;
typedef CDT::Point													Point;
typedef CGAL::Polygon_2<Gt>											Polygon_2;

bool triangle_contains_segment(Triangle t, std::set<Point3>::iterator a, std::set<Point3>::iterator b) {
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
	std::set<Point3> &vertices,
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

  std::set<CDT::Vertex_handle> points;
  // Create a unique list of vertices in this CDT and create the triangle
  for (CDT::Finite_faces_iterator fit = cdt.finite_faces_begin();
    fit != cdt.finite_faces_end(); ++fit) {
    if (fit->info().in_domain()) {
      Triangle t;
      std::pair<std::set<Point3>::iterator, bool> ret;
      ret = vertices.insert(Point3(fit->vertex(0)->point().x(), fit->vertex(0)->point().y(), fit->vertex(0)->point().z()));
      fit->vertex(0)->info() = ret.first;
      t.v0 = fit->vertex(0)->info();
      ret = vertices.insert(Point3(fit->vertex(1)->point().x(), fit->vertex(1)->point().y(), fit->vertex(1)->point().z()));
      fit->vertex(1)->info() = ret.first;
      t.v1 = fit->vertex(0)->info();
      ret = vertices.insert(Point3(fit->vertex(2)->point().x(), fit->vertex(2)->point().y(), fit->vertex(2)->point().z()));
      fit->vertex(2)->info() = ret.first;
      t.v2 = fit->vertex(0)->info();
      triangles.push_back(t);
    }
  }

	return true;
}