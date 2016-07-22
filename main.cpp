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


//-- TODO: create the topo DS locally? to prevent cases where nodes on only in one polygon. Or pprepair before?
//-- TODO: write output all polygons once the tile completely containing is closed?
//-- TODO : how to make roads horizontal "in the width"? 

//-----------------------------------------------------------------------------

#include "yaml-cpp/yaml.h"
#include "definitions.h"
#include "io.h"
#include "TopoFeature.h"
#include "Map3d.h"
#include "boost/locale.hpp"
#include <chrono>

bool validate_yaml(const char* arg, std::set<std::string>& allowedFeatures);

int main(int argc, const char * argv[]) {
  auto startTime = std::chrono::high_resolution_clock::now();
  boost::locale::generator gen;
  std::locale loc = gen("en_US.UTF-8");
  std::locale::global(loc);
  std::clog.imbue(loc);

//-- reading the config file
  if (argc != 2) {
    std::cerr << "ERROR: the config file (*.yml) is not defined." << std::endl;
    return 0;
  }
  std::clog << "Reading and validating config file: " << argv[1] << std::endl;

//-- allowed feature classes
  std::set<std::string> allowedFeatures;
  allowedFeatures.insert("Building");
  allowedFeatures.insert("Water");
  allowedFeatures.insert("Terrain");
  allowedFeatures.insert("Road");
  allowedFeatures.insert("Forest");
  allowedFeatures.insert("Separation");
  allowedFeatures.insert("Bridge/Overpass");

//-- validate the YAML file right now, nicer for the user
 if (validate_yaml(argv[1], allowedFeatures) == false) {
   std::cerr << "ERROR: config file (*.yml) is not valid. Aborting." << std::endl;
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
  if (n["Forest"]) 
    if (n["Forest"]["simplification"])
      map3d.set_forest_simplification(n["Forest"]["simplification"].as<int>());
  if (n["Water"]) 
    if (n["Water"]["height"])
      map3d.set_water_heightref(n["Water"]["height"].as<std::string>());
  if (n["Road"]) 
    if (n["Road"]["height"])
      map3d.set_road_heightref(n["Road"]["height"].as<std::string>());
  if (n["Separation"])
    if (n["Separation"]["height"])
      map3d.set_separation_heightref(n["Separation"]["height"].as<std::string>());
  if (n["Bridge/Overpass"])
    if (n["Bridge/Overpass"]["height"])
      map3d.set_bridge_heightref(n["Bridge/Overpass"]["height"].as<std::string>());

  //-- add the polygons to the map3d
  std::vector<PolygonFile> files;
  n = nodes["input_polygons"];
  bool wentgood = false;
  for (auto it = n.begin(); it != n.end(); ++it) {
    // Get the correct uniqueid attribute
    std::string uniqueid = "fid";
    if ((*it)["uniqueid"]) {
      uniqueid = (*it)["uniqueid"].as<std::string>();
    }
    // Get the correct height attribute
    std::string heightfield = "hoogtenive";
    if ((*it)["height_field"]) {
      heightfield = (*it)["height_field"].as<std::string>();
    }
    // Get the handle_multiple_heights setting
    bool handle_multiple_heights = false;
    if ((*it)["handle_multiple_heights"] && (*it)["handle_multiple_heights"].as<std::string>() == "true") {
      handle_multiple_heights = true;
    }

    // Get all datasets
    YAML::Node datasets = (*it)["datasets"];
    for (auto it2 = datasets.begin(); it2 != datasets.end(); ++it2) {
      PolygonFile file;
      file.filename = it2->as<std::string>();
      file.idfield = uniqueid;
      file.heightfield = heightfield;
      file.handle_multiple_heights = handle_multiple_heights;
      if ((*it)["lifting"]) {
        file.layers.emplace_back(std::string(), (*it)["lifting"].as<std::string>());
        files.push_back(file);
      }
      else if ((*it)["lifting_per_layer"]) {
        YAML::Node layers = (*it)["lifting_per_layer"];
        for (auto it3 = layers.begin(); it3 != layers.end(); ++it3) {
          file.layers.emplace_back(it3->first.as<std::string>(), it3->second.as<std::string>());
        }
        files.push_back(file);
      }
    }
  }

  map3d.add_polygons_files(files);
  std::clog << "\nTotal # of polygons: " << boost::locale::as::number << map3d.get_num_polygons() << std::endl;
  
  //-- spatially index the polygons
  map3d.construct_rtree();

  //-- add elevation datasets
  n = nodes["input_elevation"];
  bool bElevData = false;
  for (auto it = n.begin(); it != n.end(); ++it) {
    YAML::Node tmp = (*it)["omit_LAS_classes"];
    std::vector<int> lasomits;
    for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) 
      lasomits.push_back(it2->as<int>());
    tmp = (*it)["datasets"];
    for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
      bElevData = true;
      if ((*it)["thinning"]) 
        map3d.add_las_file(it2->as<std::string>(), lasomits, (*it)["thinning"].as<int>());
      else
        map3d.add_las_file(it2->as<std::string>(), lasomits, 1);
    }
  }

  if (bElevData == false) {
    std::cerr << "ERROR: No elevation dataset given, cannot 3dfy the dataset. Aborting." << std::endl;
    return 0;
  }

  n = nodes["options"];
  if (n["radius_vertex_elevation"])
    map3d.set_radius_vertex_elevation(n["radius_vertex_elevation"].as<float>());
  if (n["threshold_jump_edges"])
    map3d.set_threshold_jump_edges(n["threshold_jump_edges"].as<float>());

  n = nodes["output"];
  std::clog << "Lifting all input polygons to 3D..." << std::endl;
  if (n["format"].as<std::string>() == "CSV-BUILDINGS")
    map3d.threeDfy(false);
  if (n["format"].as<std::string>() == "OBJ-BUILDINGS")
    map3d.threeDfy_building_volume();
  else
    map3d.threeDfy();
  std::clog << "done." << std::endl;
  
  
  //-- output
  if (n["building_floor"].as<std::string>() == "true") 
    map3d.set_building_include_floor(true);
  int z_exaggeration = 0;
  if (n["vertical_exaggeration"]) 
    z_exaggeration = n["vertical_exaggeration"].as<int>();
  if (n["format"].as<std::string>() == "CityGML") {
    std::clog << "CityGML output" << std::endl;
    std::cout << map3d.get_citygml() << std::endl;
  }
  else if (n["format"].as<std::string>() == "OBJ") {
    std::clog << "OBJ output" << std::endl;
    std::cout << map3d.get_obj_per_feature(z_exaggeration) << std::endl;
  }
  else if (n["format"].as<std::string>() == "OBJ-NoID") {
    std::clog << "OBJ (without IDs) output" << std::endl;
    std::cout << map3d.get_obj_per_class(z_exaggeration) << std::endl;
  }
  else if (n["format"].as<std::string>() == "CSV-BUILDINGS") {
    std::clog << "CSV output (only of the buildings)" << std::endl;
    std::cout << map3d.get_csv_buildings() << std::endl;
  }
  else if (n["format"].as<std::string>() == "OBJ-BUILDINGS") {
    std::clog << "OBJ output (only of the buildings)" << std::endl;
    std::cout << map3d.get_obj_building_volume(z_exaggeration) << std::endl;
  }
  else if (n["format"].as<std::string>() == "Shapefile") {
    std::clog << "Shapefile output" << std::endl;
    std::string filename = n["filename"].as<std::string>();
    if (map3d.get_shapefile(filename)) {
      std::clog << "Shapefile written" << std::endl;
    }
    else
    {
      std::cerr << "Writing shapefile failed" << std::endl;
      return 0;
    }
  }

  //-- bye-bye
  long totalTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - startTime).count();
  std::clog << "Successfully terminated in " << totalTime;
  int hours = totalTime / 3600;
  totalTime -= hours * 3600;
  int minutes = totalTime / 60;
  totalTime -= minutes * 60;
  int seconds = totalTime;
  std::clog << " seconds || " << hours <<  ":" << minutes << ":" << seconds <<"." << std::endl;
  return 1;
}


bool validate_yaml(const char* arg, std::set<std::string>& allowedFeatures) {
  YAML::Node nodes = YAML::LoadFile(arg);
  bool wentgood = true;
//-- 1. input polygons classes
  YAML::Node n = nodes["input_polygons"];
  for (auto it = n.begin(); it != n.end(); ++it) {
    if ((*it)["lifting_per_layer"]) {
      YAML::Node tmp = (*it)["lifting_per_layer"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
        if (allowedFeatures.count((it2->second).as<std::string>()) == 0) {
          std::cerr << "\tLifting class '" << (it2->second).as<std::string>() << "' unknown." << std::endl;
          wentgood = false;
        }
      }
    }
    else if ((*it)["lifting"]) {
      YAML::Node tmp = (*it)["datasets"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
        if (allowedFeatures.count((*it)["lifting"].as<std::string>()) == 0) {
          std::cerr << "\tLifting class '" << (*it)["lifting"].as<std::string>() << "' unknown." << std::endl;
          wentgood = false;
        }
      }
    }
  }  
//-- 2. lifting_options
  n = nodes["lifting_options"];
  if (n["Building"]) {
    if (n["Building"]["height_roof"]) {
      std::string s = n["Building"]["height_roof"].as<std::string>();
      if ( (s.substr(0, s.find_first_of("-")) != "percentile") ||
          (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false) ) {
        wentgood = false;
        std::cerr << "\tOption 'Building.height_roof' invalid; must be 'percentile-XX'." << std::endl;
      }
    }
    if (n["Building"]["height_ground"]) {
      std::string s = n["Building"]["height_ground"].as<std::string>();
      if ( (s.substr(0, s.find_first_of("-")) != "percentile") ||
          (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false) ) {
        wentgood = false;
        std::cerr << "\tOption 'Building.height_ground' invalid; must be 'percentile-XX'." << std::endl;
      }
    }
    if (n["Building"]["triangulate"]) {
      std::string s = n["Building"]["triangulate"].as<std::string>();
      if ( (s != "true") && (s != "false") ) {
        wentgood = false;
        std::cerr << "\tOption 'Building.triangulate' invalid; must be 'true' or 'false'." << std::endl;
      }
    }
  }
  if (n["Water"]) {
    if (n["Water"]["height"]) {
      std::string s = n["Water"]["height"].as<std::string>();
      if ( (s.substr(0, s.find_first_of("-")) != "percentile") ||
          (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false) ) {
        wentgood = false;
        std::cerr << "\tOption 'Water.height' invalid; must be 'percentile-XX'." << std::endl;
      }
    }
  }  
  if (n["Road"]) {
    if (n["Road"]["height"]) {
      std::string s = n["Road"]["height"].as<std::string>();
      if ( (s.substr(0, s.find_first_of("-")) != "percentile") ||
          (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false) ) {
        wentgood = false;
        std::cerr << "\tOption 'Road.height' invalid; must be 'percentile-XX'." << std::endl;
      }
    }
  }
  if (n["Terrain"]) {
    if (n["Terrain"]["simplification"]) {
      if (is_string_integer(n["Terrain"]["simplification"].as<std::string>()) == false) {
        wentgood = false;
        std::cerr << "\tOption 'Terrain.simplification' invalid; must be an integer." << std::endl;
      }
    }
  }  
  if (n["Forest"]) {
    if (n["Forest"]["simplification"]) {
      if (is_string_integer(n["Forest"]["simplification"].as<std::string>()) == false) {
        wentgood = false;
        std::cerr << "\tOption 'Forest.simplification' invalid; must be an integer." << std::endl;
      }
    }
  }
  if (n["Separation"]) {
    if (n["Separation"]["height"]) {
      std::string s = n["Separation"]["height"].as<std::string>();
      if ((s.substr(0, s.find_first_of("-")) != "percentile") ||
        (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false)) {
        wentgood = false;
        std::cerr << "\tOption 'Separation.height' invalid; must be 'percentile-XX'." << std::endl;
      }
    }
  }
  if (n["Bridge/Overpass"]) {
    if (n["Bridge/Overpass"]["height"]) {
      std::string s = n["Bridge/Overpass"]["height"].as<std::string>();
      if ((s.substr(0, s.find_first_of("-")) != "percentile") ||
        (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false)) {
        wentgood = false;
        std::cerr << "\tOption 'Bridge/Overpass.height' invalid; must be 'percentile-XX'." << std::endl;
      }
    }
  }
//-- 3. input_elevation
  n = nodes["input_elevation"];
  for (auto it = n.begin(); it != n.end(); ++it) {
    YAML::Node tmp = (*it)["omit_LAS_classes"];
    for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
      if (is_string_integer(it2->as<std::string>()) == false) {
        wentgood = false;
        std::cerr << "\tOption 'input_elevation.omit_LAS_class' invalid; must be an integer." << std::endl;
      }
    }
    if ((*it)["thinning"]) {
      if (is_string_integer((*it)["thinning"].as<std::string>()) == false) {
        wentgood = false;
        std::cerr << "\tOption 'input_elevation.thinning' invalid; must be an integer." << std::endl;
      }
    }
  }
//-- 4. options
  n = nodes["options"];
  if (n["radius_vertex_elevation"]) {
    try {
      boost::lexical_cast<float>(n["radius_vertex_elevation"].as<std::string>());
    }
    catch(boost::bad_lexical_cast& e) {
      wentgood = false;
      std::cerr << "\tOption 'options.radius_vertex_elevation' invalid." << std::endl;
    }
  }
  if (n["threshold_jump_edges"]) {
    try {
      boost::lexical_cast<float>(n["threshold_jump_edges"].as<std::string>());
    }
    catch(boost::bad_lexical_cast& e) {
      wentgood = false;
      std::cerr << "\tOption 'options.threshold_jump_edges' invalid." << std::endl;
    }
  }
//-- 5. output
  n = nodes["output"];
  if ( (n["format"].as<std::string>() != "OBJ") &&
       (n["format"].as<std::string>() != "OBJ-NoID") &&
       (n["format"].as<std::string>() != "CityGML") &&
       (n["format"].as<std::string>() != "OBJ-BUILDINGS") &&
       (n["format"].as<std::string>() != "CSV-BUILDINGS")  &&
       (n["format"].as<std::string>() != "Shapefile") ) {
    wentgood = false;
    std::cerr << "\tOption 'output.format' invalid (OBJ | OBJ-NoID | CityGML | CSV-BUILDINGS | Shapefile)" << std::endl;
  }
  //-- Shapefile type filename check
  if (n["format"].as<std::string>() == "Shapefile" && !n["filename"]) {
    wentgood = false;
    std::cerr << "\tOption 'output.format' Shapefile needs an output.filename" << std::endl;
  }
  return wentgood;
}
