
#include "definitions.h"
#include "io.h"
#include "TopoFeature.h"
#include "Map3d.h"
#include "yaml-cpp/yaml.h"


int main(int argc, const char * argv[]) {
  
  if (argc != 2) {
    std::cout << "Error: the config file (*.yml) is not defined." << std::endl;
    return 0;
  }
  
  Map3d map3d;
  YAML::Node nodes = YAML::LoadFile(argv[1]);
  
  //-- add the polygons
  YAML::Node n = nodes["input_polygons"];
  for (auto it = n.begin(); it != n.end(); ++it) {
    if ((*it)["layers"]) {
      YAML::Node tmp = (*it)["layers"];
      std::vector<std::pair<std::string, std::string> > layers;
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
        std::pair<std::string, std::string> onepair( (it2->first).as<std::string>(), (it2->second).as<std::string>() );
        layers.push_back(onepair);
      }
      tmp = (*it)["datasets"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
        map3d.add_polygons_file(it2->as<std::string>(),
                                (*it)["uniqueid"].as<std::string>(),
                                layers);
      }
    }
    else if ((*it)["lifting"]) {
      YAML::Node tmp = (*it)["datasets"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
        map3d.add_polygons_file(it2->as<std::string>(),
                                (*it)["uniqueid"].as<std::string>(),
                                (*it)["lifting"].as<std::string>());
      }
    }
  }
  std::clog << "\nTotal # of polygons: " << map3d.get_num_polygons() << std::endl;
  
  //-- spatially index the polygons
  map3d.construct_rtree();

  //-- add elevation datasets
  n = nodes["input_elevation"];
  for (auto it = n.begin(); it != n.end(); ++it) {
    YAML::Node tmp = (*it)["datasets"];
    for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
      map3d.add_las_file(it2->as<std::string>(), (*it)["skip"].as<int>());
    }
  }

  map3d.threeDfy();
  
  n = nodes["output"];
  if (n["building_floor"].as<std::string>() == "true") 
    map3d._buildingfloor = true;
  if (n["format"].as<std::string>() == "CityGML") {
    std::clog << "CityGML output" << std::endl;
    std::cout << map3d.get_citygml() << std::endl;
  }
  else if (n["format"].as<std::string>() == "CSV") {
    std::clog << "CSV output" << std::endl;
    std::cout << map3d.get_csv() << std::endl;
  }
  else if (n["format"].as<std::string>() == "OBJ") {
    std::clog << "OBJ output" << std::endl;
    std::cout << map3d.get_obj() << std::endl;
  }

  std::clog << "Successfully terminated." << std::endl;
  return 1;
}


