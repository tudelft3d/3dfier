
/*
 ___   _ ___ _         ___     _       
|_  |_| |  _|_|___ ___|_  |___|_|_____ 
|_  | . |  _| | -_|  _|  _|_ -| |     |
|___|___|_| |_|___|_| |___|___|_|_|_|_|
                                       

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

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>
#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>

#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>

#include <iostream>
#include <fstream>
 
namespace PMP = CGAL::Polygon_mesh_processing;

struct FaceInfo2
{
  FaceInfo2(){}
  int nesting_level;
  bool in_domain(){ 
    return nesting_level%2 == 1;
  }
};

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

typedef K::Point_2    Point2;
typedef K::Point_3    Point3;

typedef CGAL::Surface_mesh<Point3>                SMesh;
typedef SMesh::vertex_index                       vertex_index;
typedef SMesh::Face_index                         face_descriptor;

typedef CGAL::Triangulation_vertex_base_with_info_2 <vertex_index,K>      Vb;
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2,K>            Fbb;
typedef CGAL::Constrained_triangulation_face_base_2<K,Fbb>                Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb>                       TDS;
typedef CGAL::Exact_predicates_tag                                        Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>          CDT;


          
//explore set of facets connected with non constrained edges,
//and attribute to each such set a nesting level.
//We start from facets incident to the infinite vertex, with a nesting
//level of 0. Then we recursively consider the non-explored facets incident 
//to constrained edges bounding the former set and increase the nesting level by 1.
//Facets in the domain are those with an odd nesting level.
void 
mark_domains(CDT& ct, 
             CDT::Face_handle start, 
             int index, 
             std::list<CDT::Edge>& border )
{
  if(start->info().nesting_level != -1){
    return;
  }
  std::list<CDT::Face_handle> queue;
  queue.push_back(start);
  while(! queue.empty()){
    CDT::Face_handle fh = queue.front();
    queue.pop_front();
    if(fh->info().nesting_level == -1){
      fh->info().nesting_level = index;
      for(int i = 0; i < 3; i++){
        CDT::Edge e(fh,i);
        CDT::Face_handle n = fh->neighbor(i);
        if(n->info().nesting_level == -1){
          if(ct.is_constrained(e)) border.push_back(e);
          else queue.push_back(n);
        }
      }
    }
  }
}


void mark_domains(CDT& cdt)
{
  for(CDT::All_faces_iterator it = cdt.all_faces_begin(); it != cdt.all_faces_end(); ++it){
    it->info().nesting_level = -1;
  }
  std::list<CDT::Edge> border;
  mark_domains(cdt, cdt.infinite_face(), 0, border);
  while(! border.empty()){
    CDT::Edge e = border.front();
    border.pop_front();
    CDT::Face_handle n = e.first->neighbor(e.second);
    if(n->info().nesting_level == -1){
      mark_domains(cdt, n, e.first->info().nesting_level+1, border);
    }
  }
}

 
int main(int argc, char* argv[])
{
  const char* filename = (argc > 1) ? argv[1] : "a_cleaned.off";
  std::ifstream input(filename);
  SMesh mesh;
  if (!input || !(input >> mesh) || mesh.is_empty()) {
    std::cerr << "Not a valid off file." << std::endl;
    return 1;
  }
 
  std::cout << "=====INPUT=====" << std::endl;
  std::cout << "# vertices: " << mesh.number_of_vertices() << std::endl;
  std::cout << "# faces: " << mesh.number_of_faces() << std::endl;
  std::cout << "is_valid? " << mesh.is_valid() << std::endl;
  std::cout << "is_closed? " << CGAL::is_closed(mesh) << std::endl;
  std::cout << std::endl;

  //-- calculate the bbox
  double minx = 9e10;
  double miny = 9e10;
  double maxx = -9e10;
  double maxy = -9e10;
  for(vertex_index vd : mesh.vertices()) {
    if (mesh.point(vd).x() < minx)
      minx = mesh.point(vd).x();
    if (mesh.point(vd).y() < miny)
      miny = mesh.point(vd).y();
    if (mesh.point(vd).x() > maxx)
      maxx = mesh.point(vd).x();
    if (mesh.point(vd).y() > maxy)
      maxy = mesh.point(vd).y();
  }
  minx -= 200;
  miny -= 200;
  maxx += 200;
  maxy += 200;
  std::cout << "bbox min (" << minx << ", " << miny << ")" << std::endl;
  std::cout << "bbox max (" << maxx << ", " << maxy << ")" << std::endl;

  //-- construct a CDT and link vertices of the SMesh
  std::cout << "--construct CDT" << std::endl;
  CDT cdt;
  std::vector<vertex_index> bboxi;
  bboxi.push_back(mesh.add_vertex(Point3(minx, miny, 0)));
  CDT::Vertex_handle va = cdt.insert(Point2(minx, miny));
  va->info() = bboxi.back();
  bboxi.push_back(mesh.add_vertex(Point3(maxx, miny, 0)));
  CDT::Vertex_handle vb = cdt.insert(Point2(maxx, miny));
  vb->info() = bboxi.back();
  bboxi.push_back(mesh.add_vertex(Point3(maxx, maxy, 0)));
  CDT::Vertex_handle vc = cdt.insert(Point2(maxx, maxy));
  vc->info() = bboxi.back();
  bboxi.push_back(mesh.add_vertex(Point3(minx, maxy, 0)));
  CDT::Vertex_handle vd = cdt.insert(Point2(minx, maxy));
  vd->info() = bboxi.back();
  //-- construct the constrained edges of the bbox
  cdt.insert_constraint(va, vb);
  cdt.insert_constraint(vb, vc);
  cdt.insert_constraint(vc, vd);
  cdt.insert_constraint(vd, va);
  
  //-- find the border edges in the SMesh
  for(auto e : mesh.edges()) {
    if (mesh.is_border(e) == true) {
      vertex_index a = mesh.vertex(e, 0);
      CDT::Vertex_handle vha = cdt.insert(Point2(mesh.point(a).x(), mesh.point(a).y()));
      vha->info() = a;
      vertex_index b = mesh.vertex(e, 1);
      CDT::Vertex_handle vhb = cdt.insert(Point2(mesh.point(b).x(), mesh.point(b).y()));
      vhb->info() = b;
      if (vha != vhb)
        cdt.insert_constraint(vha, vhb);
      // break;
    }
  }

  mark_domains(cdt);
  CDT::Finite_faces_iterator fi = cdt.finite_faces_begin();
  for( ; fi != cdt.finite_faces_end(); fi++)
  {
    if ( fi->info().in_domain() )
    {
      if (mesh.add_face(fi->vertex(0)->info(), fi->vertex(1)->info(), fi->vertex(2)->info()) == SMesh::null_face())
      {
        std::cout << " -- FLIPPED -- " << std::endl;
        if (mesh.add_face(fi->vertex(0)->info(), fi->vertex(2)->info(), fi->vertex(1)->info()) == SMesh::null_face())
          std::cout << "IMPOSSIBLE TO INSERT FACE " << std::endl;
      }
    }
  }

  //-- close the 'bottom' of the model
  std::cout << "--close bottom of model" << std::endl;
  std::vector<vertex_index> bboxi2;
  bboxi2.push_back(mesh.add_vertex(Point3(minx, miny, -100)));
  bboxi2.push_back(mesh.add_vertex(Point3(maxx, miny, -100)));
  bboxi2.push_back(mesh.add_vertex(Point3(maxx, maxy, -100)));
  bboxi2.push_back(mesh.add_vertex(Point3(minx, maxy, -100)));
  
  for (int i = 0; i < (bboxi.size() - 1); i++)
  {
    mesh.add_face(bboxi[i],  bboxi2[i], bboxi[i+1]);
    mesh.add_face(bboxi2[i], bboxi2[i+1], bboxi[i+1]);
  }
  mesh.add_face(bboxi.back(),  bboxi2.back(),  bboxi.front());
  mesh.add_face(bboxi2.back(), bboxi2.front(), bboxi.front());
  //-- bottom face
  mesh.add_face(bboxi2[0], bboxi2[3], bboxi2[2]);
  mesh.add_face(bboxi2[0], bboxi2[2], bboxi2[1]);


  //-- fill holes
  std::cout << "--fill holes in model" << std::endl;
  std::vector<face_descriptor> patch_facets;
  for(auto hed : mesh.halfedges()) {
    if (mesh.is_border(hed) == true) {
      std::cout << "he border" << std::endl;
      PMP::triangulate_hole(mesh, hed, std::back_inserter(patch_facets));
      std::cout << " Number of facets in constructed patch: " << patch_facets.size() << std::endl;
      // for (auto each : patch_facets)
      //   std::cout << each << std::endl;
      // break;
    }
  } 
 

  std::cout << "=====OUTPUT=====" << std::endl;
  std::cout << "# vertices: " << mesh.number_of_vertices() << std::endl;
  std::cout << "# faces: " << mesh.number_of_faces() << std::endl;
  std::cout << "is_valid? " << mesh.is_valid() << std::endl;
  std::cout << "is_closed? " << CGAL::is_closed(mesh) << std::endl;
  std::cout << std::endl;
  //-- check for self-intersections
  if (PMP::does_self_intersect(mesh) == true)
    std::cout << "self-intersection? yes" << std::endl;
  else
    std::cout << "self-intersection? no" << std::endl;
 
 
  //-- save the updated SMesh in OFF
  std::cout << "output mesh saved to out.off" << std::endl;
  std::ofstream out("out.off");
  out << mesh;
  out.close(); 
 
  return (0);
}