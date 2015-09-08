
#include "definitions.h"
#include "input.h"
#include "Polygon3d.h"
#include "Map3d.h"
#include "yaml-cpp/yaml.h"


int main(int argc, const char * argv[]) {
  
  if (argc != 2) {
    std::cout << "Error: the config file (*.yml) is not defined." << std::endl;
    return 0;
  }
  
  Map3d map3d;
  
  YAML::Node nodes = YAML::LoadFile(argv[1]);
  
  YAML::Node n = nodes["input_polygons"];
  for (auto it = n.begin(); it != n.end(); ++it) {
    if ((*it)["layers"]) {
      std::cout << "lll: " << it->size() << std::endl;
      YAML::Node tmp = (*it)["layers"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
        std::cout << it2->first << " : " << it2->second << std::endl;
    }
    else if ((*it)["lifting"]) {
      YAML::Node tmp = (*it)["datasets"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
        map3d.add_polygons_file(it2->as<std::string>(),
                                (*it)["uniqueid"].as<std::string>(),
                                (*it)["uniqueid"].as<std::string>());
      }
    }
  }

//  n = nodes["output"];
//  std::cout << n.Type() << std::endl;
//  switch (n.Type()) {
//      case YAML::NodeType::Null: // ...
//          std::cout << "null" << std::endl;
//      case YAML::NodeType::Scalar: // ...
//          std::cout << "scalar" << std::endl;
//      case YAML::NodeType::Sequence: // ...
//          std::cout << "sequence" << std::endl;
//      case YAML::NodeType::Map: // ...
//          std::cout << "map" << std::endl;
//          std::cout << n.size() << std::endl;
//          for (auto it = n.begin(); it != n.end(); it++) {
//             std::cout << it->first << std::endl;
//          }
//          break;
//      case YAML::NodeType::Undefined: // ...
//          std::cout << "fuck knows" << std::endl;
//  }
//  std::cout << "\n=========\n" << std::endl;
  


  std::cout << "done." << std::endl;
  n = nodes["input_elevation"];
  std::cout << n.size() << std::endl;
  std::cout << n[0] << std::endl;

  return 1;
//  Map3d map3d;

  std::vector< std::pair<std::string, Lift3DType> > layers;

  // map3d.add_allowed_layer("Terrein");
  // map3d.add_allowed_layer("Waterdeel");
  // map3d.add_allowed_layer("Wegdeel");
  // map3d.add_allowed_layer("Gebouw");
  // map3d.add_allowed_layer("Gebouw_Vlak");
  //  map3d.add_gml_file("/Users/hugo/data/top10nl/nlextract/TOP10NL_37W_00.gml", "identificatie");
  // map3d.add_shp_file("/Users/hugo/data/campus/partof.shp", "FACE_ID");

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







