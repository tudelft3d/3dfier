
#include "definitions.h"
#include "input.h"
#include "Polygon3d.h"
#include "Map3d.h"



int main(int argc, const char * argv[]) {
  Map3d map3d;
  map3d.add_allowed_layer("Terrein");
  map3d.add_allowed_layer("Waterdeel");
  map3d.add_allowed_layer("Wegdeel");
  map3d.add_allowed_layer("Gebouw");
  map3d.add_allowed_layer("Gebouw_Vlak");
  
//  map3d.add_gml_file("/Users/hugo/data/top10nl/nlextract/TOP10NL_37W_00.gml", "identificatie");
  map3d.add_shp_file("/Users/hugo/data/campus/partof.shp", "FACE_ID");

  std::clog << "# of polygons: " << map3d.get_num_polygons() << std::endl;
  map3d.construct_rtree();

  map3d.add_las_file("/Users/hugo/data/campus/coverpartof.laz");
  // map3d.add_las_file("/Users/hugo/data/ahn2/u37bn2.laz");
  // map3d.add_las_file("/Users/hugo/data/ahn2/u37en2.laz");
  // map3d.add_point(74659.1, 447682.4, 11.1);

  std::cout << map3d.get_citygml() << std::endl;
  

  //-- write string to pugi object for validation/formatting
//  pugi::xml_document doc;
//  doc.load_string("<foo bar='baz'><call>hey</call></foo>");
//  std::stringstream ss;
//  doc.save(ss);
//  std::cout << "Hugo" << ss.str() << std::endl;

//  const std::vector<Polygon3d*> l = map3d.get_polygons3d();
//  for (auto p: l) {
//    if (p->get_3d_representation() != "")
//      std::cout << p->get_3d_representation() << std::endl;
//  }
  std::clog << "done." << std::endl;
  return 1;
}







