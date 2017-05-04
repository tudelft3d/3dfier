
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>
#include <CGAL/Polygon_mesh_processing/orientation.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>

#include <CGAL/Triangulation_vertex_base_with_info_2.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>

#include <iostream>
#include <fstream>
 
namespace PMP = CGAL::Polygon_mesh_processing;

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;

typedef K::Point_2    Point2;
typedef K::Point_3    Point3;
// typedef K::Vector_3   Vector3;

typedef CGAL::Surface_mesh<Point3>                            SMesh;
typedef SMesh::vertex_index                                   vertex_index;
// typedef boost::graph_traits<SMesh>::face_descriptor    face_descriptor;

typedef CGAL::Triangulation_vertex_base_with_info_2 <vertex_index,K>        Vb;
typedef CGAL::Constrained_triangulation_face_base_2<K>                      Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb>                         TDS;
typedef CGAL::Exact_intersections_tag                                       Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>            CDT;
          

 
int main(int argc, char* argv[])
{
  // const char* filename = (argc > 1) ? argv[1] : "a.off";
  const char* filename = (argc > 1) ? argv[1] : "a_cleaned.off";
  std::ifstream input(filename);
  SMesh mesh;
  if (!input || !(input >> mesh) || mesh.is_empty()) {
    std::cerr << "Not a valid off file." << std::endl;
    return 1;
  }
 
  std::cout << "# vertices: " << mesh.number_of_vertices() << std::endl;
  std::cout << "# faces: " << mesh.number_of_faces() << std::endl;

  std::cout << "is_closed? " << CGAL::is_closed(mesh) << std::endl;
  std::cout << "is_valid? " << mesh.is_valid() << std::endl;

  //-- find the bbox
  double tmpx, tmpy;
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
  std::cout << "minx (" << minx << ", " << miny << ")" << std::endl;
  std::cout << "maxx (" << maxx << ", " << maxy << ")" << std::endl;

  //-- construct a CDT and link vertices of the SMesh
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

  std::cout << "# vertices:  " << cdt.number_of_vertices() << std::endl;
  std::cout << "# triangles: " << cdt.number_of_faces() << std::endl;

  // CDT::Finite_vertices_iterator vit;
  // for (vit = cdt.finite_vertices_begin(); vit != cdt.finite_vertices_end(); ++vit) {
    // std::cout << vit->info() << std::endl;
  // }

  // if (mesh.add_face(bboxi[0], bboxi[1], bboxi[2]) == SMesh::null_face())
        // std::cout << "IMPOSSIBLE TO INSERT FACE " << std::endl;

  // std::cout << "-----" << std::endl;
  CDT::Finite_faces_iterator fi = cdt.finite_faces_begin();
  for( ; fi != cdt.finite_faces_end(); fi++)
  {
    if ( (std::find(bboxi.begin(), bboxi.end(), fi->vertex(0)->info()) != bboxi.end()) ||
         (std::find(bboxi.begin(), bboxi.end(), fi->vertex(1)->info()) != bboxi.end()) ||
         (std::find(bboxi.begin(), bboxi.end(), fi->vertex(2)->info()) != bboxi.end()) )
    {
      std::cout << fi->vertex(0)->info() << " | ";
      std::cout << fi->vertex(1)->info() << " | ";
      std::cout << fi->vertex(2)->info() << std::endl;

      if (mesh.add_face(fi->vertex(0)->info(), fi->vertex(1)->info(), fi->vertex(2)->info()) == SMesh::null_face())
      {
        mesh.add_face(fi->vertex(0)->info(), fi->vertex(2)->info(), fi->vertex(1)->info());
        std::cout << " -- FLIPPED -- " << std::endl;
        // std::cout << "IMPOSSIBLE TO INSERT FACE " << std::endl;
      }
    }
  }
    


  // if (PMP::does_self_intersect(mesh) == true)
  //   std::cout << "self-intersection? yes" << std::endl;
  // else
  //   std::cout << "self-intersection? no" << std::endl;
 
  // if (CGAL::Polygon_mesh_processing::is_outward_oriented(mesh) == true)
  //   std::cout << "OUTWARDS" << std::endl;
  // else
  //   std::cout << "INWARDS" << std::endl;
 
  //-- save the updated SMesh in OFF
  std::ofstream out("out.off");
  out << mesh;
  out.close(); 
 
  return (1);
}