
//-- TODO: add GDAL2.0 support
//-- TODO: filter the noise in the lidar points inside TIN?
//-- TODO : how to make roads horizontal "in the length"? or do we need to?

//-----------------------------------------------------------------------------

#include "definitions.h"
#include "io.h"
#include "TopoFeature.h"
#include "Map3d.h"
#include "yaml-cpp/yaml.h"


int main(int argc, const char * argv[]) {
  
//-- allowed feature classes
  std::set<std::string> allowedFeatures;
  allowedFeatures.insert("Building");
  allowedFeatures.insert("Water");
  allowedFeatures.insert("Terrain");
  allowedFeatures.insert("Road");
  allowedFeatures.insert("Vegetation");
  allowedFeatures.insert("Bridge/Overpass");

//-- reading the config file
    std::clog << "Reading config file: " << argv[1] << std::endl;
  if (argc != 2) {
    std::cerr << "ERROR: the config file (*.yml) is not defined." << std::endl;
    return 0;
  }
  
  Map3d map3d;
  YAML::Node nodes = YAML::LoadFile(argv[1]);
  
//-- store the lifting options in the Map3d
  YAML::Node n = nodes["lifting_options"];
  if (n["Building"]) {
    if (n["Building"]["height_roof"])
      map3d.set_building_heightref_roof(n["Building"]["height_roof"].as<std::string>());
    if (n["Building"]["height_floor"])
      map3d.set_building_heightref_floor(n["Building"]["height_floor"].as<std::string>());
    if (n["Building"]["triangulate"]) {
      if (n["Building"]["triangulate"].as<std::string>() == "true") 
        map3d.set_building_triangulate(true);
      else
        map3d.set_building_triangulate(false);
    }
  }
  if (n["Terrain"]) 
    if (n["Terrain"]["simplification"])
      map3d.set_terrain_simplification(n["Terrain"]["simplification"].as<int>());
  if (n["Vegetation"]) 
    if (n["Vegetation"]["simplification"])
      map3d.set_vegetation_simplification(n["Vegetation"]["simplification"].as<int>());
  if (n["Water"]) 
    if (n["Water"]["height"])
      map3d.set_water_heightref(n["Water"]["height"].as<std::string>());
  if (n["Road"]) 
    if (n["Road"]["height"])
      map3d.set_road_heightref(n["Road"]["height"].as<std::string>());

//-- add the polygons to the map3d
  n = nodes["input_polygons"];
  bool wentgood = true;
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
        wentgood = map3d.add_polygons_file(it2->as<std::string>(),
                                           (*it)["uniqueid"].as<std::string>(),
                                           layers);
      }
    }
    else if ((*it)["lifting"]) {
      YAML::Node tmp = (*it)["datasets"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
        if (allowedFeatures.count((*it)["lifting"].as<std::string>()) == 1) {
          wentgood = map3d.add_polygons_file(it2->as<std::string>(),
                                             (*it)["uniqueid"].as<std::string>(),
                                             (*it)["lifting"].as<std::string>());
        }
        else {
          std::clog << "ERROR: class '" << (*it)["lifting"].as<std::string>() << "' not recognised." << std::endl;
          wentgood = false;
        }
      }
    }
  }
  if (wentgood == false) {
    std::cerr << "Something went bad while reading input polygons. Abort." << std::endl;
    return 0;
  }

  std::clog << "\nTotal # of polygons: " << map3d.get_num_polygons() << std::endl;
  
  //-- spatially index the polygons
  map3d.construct_rtree();

  //-- add elevation datasets
  n = nodes["input_elevation"];
  for (auto it = n.begin(); it != n.end(); ++it) {
    YAML::Node tmp = (*it)["omit_LAS_classes"];
    std::vector<int> lasomits;
    for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) 
      lasomits.push_back(it2->as<int>());
    tmp = (*it)["datasets"];
    for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
      map3d.add_las_file(it2->as<std::string>(), lasomits, (*it)["thinning"].as<int>());
    }
  }

  map3d.threeDfy();
  // return 1;

  
  //-- output
  n = nodes["output"];
  if (n["building_floor"].as<std::string>() == "true") 
    map3d.set_building_include_floor(true);
  if (n["format"].as<std::string>() == "CityGML") {
    std::clog << "CityGML output" << std::endl;
    std::cout << map3d.get_citygml() << std::endl;
  }
  else if (n["format"].as<std::string>() == "OBJ") {
    std::clog << "OBJ output" << std::endl;
    std::cout << map3d.get_obj() << std::endl;
  }
  else if (n["format"].as<std::string>() == "CSV-BUILDINGS") {
    std::clog << "CSV output (only of the buildings)" << std::endl;
    std::cout << map3d.get_csv_buildings() << std::endl;
  }

  std::clog << "Successfully terminated." << std::endl;
  return 1;
}


