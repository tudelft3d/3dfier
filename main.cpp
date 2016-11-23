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
#include "boost/chrono.hpp"

std::string VERSION = "0.8";

bool validate_yaml(const char* arg, std::set<std::string>& allowedFeatures);
void print_license();

int main(int argc, const char * argv[]) {
  auto startTime = boost::chrono::high_resolution_clock::now();
  boost::locale::generator gen;
  std::locale loc = gen("en_US.UTF-8");
  std::locale::global(loc);
  std::clog.imbue(loc);

  std::string licensewarning =
    "3dfier Copyright (C) 2015-2016  3D geoinformation research group, TU Delft\n"
    "This program comes with ABSOLUTELY NO WARRANTY.\n"
    "This is free software, and you are welcome to redistribute it\n"
    "under certain conditions; for details run 3dfier with the '--license' option.\n";

  std::string filename;

  //-- reading the config file
  if (argc == 2) {
    std::string s = argv[1];
    if (s == "--license") {
      print_license();
      return 0;
    }
    if (s == "--version") {
      std::clog << "3dfier " << VERSION << std::endl;
      return 0;
    }
    else {
      std::clog << licensewarning << std::endl;
      std::cerr << "Usage: 3dfier config.yml -o output.ext" << std::endl;
      return 0;
    }
  }
  else if (argc == 4 && (std::string)argv[2] == "-o") {
    filename = argv[3];
  }
  else {
    std::clog << licensewarning << std::endl;
    std::cerr << "Usage: 3dfier config.yml -o output.ext" << std::endl;
    return 0;
  }

  std::clog << licensewarning << std::endl;
  std::clog << "Reading config file: " << argv[1] << std::endl;

  //-- allowed feature classes
  std::set<std::string> allowedFeatures{"Building", "Water", "Terrain", "Road", "Forest", "Separation", "Bridge/Overpass"};

  //-- validate the YAML file right now, nicer for the user
  if (validate_yaml(argv[1], allowedFeatures) == false) {
    std::cerr << "ERROR: config file (*.yml) is not valid. Aborting." << std::endl;
    return 0;
  }
  std::clog << "Config file is valid." << std::endl;

  Map3d map3d;
  YAML::Node nodes = YAML::LoadFile(argv[1]);
  //-- store the lifting options in the Map3d
  YAML::Node n = nodes["lifting_options"];
  if (n["Building"]) {
    if (n["Building"]["height_roof"]) {
      std::string height = n["Building"]["height_roof"].as<std::string>();
      map3d.set_building_heightref_roof(std::stof(height.substr(height.find_first_of("-") + 1)) / 100);
    }
    if (n["Building"]["height_floor"]) {
      std::string height = n["Building"]["height_floor"].as<std::string>();
      map3d.set_building_heightref_floor(std::stof(height.substr(height.find_first_of("-") + 1)) / 100);
    }
    if (n["Building"]["lod"]) {
      map3d.set_building_lod(n["Building"]["lod"].as<int>());
    }
    if (n["Building"]["triangulate"]) {
      if (n["Building"]["triangulate"].as<std::string>() == "true")
        map3d.set_building_triangulate(true);
      else
        map3d.set_building_triangulate(false);
    }
  }
  if (n["Terrain"]) {
    if (n["Terrain"]["simplification"])
      map3d.set_terrain_simplification(n["Terrain"]["simplification"].as<int>());
    if (n["Terrain"]["innerbuffer"])
      map3d.set_terrain_innerbuffer(n["Terrain"]["innerbuffer"].as<float>());
  }
  if (n["Forest"]) {
    if (n["Forest"]["simplification"])
      map3d.set_forest_simplification(n["Forest"]["simplification"].as<int>());
    if (n["Forest"]["innerbuffer"])
      map3d.set_forest_innerbuffer(n["Forest"]["innerbuffer"].as<float>());
    if (n["Forest"]["ground_points_only"] && n["Forest"]["ground_points_only"].as<std::string>() == "true")
      map3d.set_forest_ground_points_only(true);
  }
  if (n["Water"]) {
    if (n["Water"]["height"]) {
      std::string height = n["Water"]["height"].as<std::string>();
      map3d.set_water_heightref(std::stof(height.substr(height.find_first_of("-") + 1)) / 100);
    }
  }
  if (n["Road"]) {
    if (n["Road"]["height"]) {
      std::string height = n["Road"]["height"].as<std::string>();
      map3d.set_road_heightref(std::stof(height.substr(height.find_first_of("-") + 1)) / 100);
    }
  }
  if (n["Separation"]) {
    if (n["Separation"]["height"]) {
      std::string height = n["Separation"]["height"].as<std::string>();
      map3d.set_separation_heightref(std::stof(height.substr(height.find_first_of("-") + 1)) / 100);
    }
  }
  if (n["Bridge/Overpass"]) {
    if (n["Bridge/Overpass"]["height"]) {
      std::string height = n["Bridge/Overpass"]["height"].as<std::string>();
      map3d.set_bridge_heightref(std::stof(height.substr(height.find_first_of("-") + 1)) / 100);
    }
  }

  n = nodes["options"];
  bool bStitching = true;
  if (n["radius_vertex_elevation"])
    map3d.set_radius_vertex_elevation(n["radius_vertex_elevation"].as<float>());
  if (n["building_radius_vertex_elevation"])
    map3d.set_building_radius_vertex_elevation(n["building_radius_vertex_elevation"].as<float>());
  if (n["threshold_jump_edges"])
    map3d.set_threshold_jump_edges(n["threshold_jump_edges"].as<float>());
  if (n["use_vertical_walls"] && n["use_vertical_walls"].as<std::string>() == "true")
    map3d.set_use_vertical_walls(true);
  if (n["stitching"]) {
    if (n["stitching"].as<std::string>() == "false")
      bStitching = false;
  }

  //-- add the polygons to the map3d
  std::vector<PolygonFile> files;
  n = nodes["input_polygons"];
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
  Box2 b = map3d.get_bbox();
  std::clog << std::setprecision(3) << std::fixed;
  std::clog << "Spatial extent: ("
    << bg::get<bg::min_corner, 0>(b) << ", "
    << bg::get<bg::min_corner, 1>(b) << ") ("
    << bg::get<bg::max_corner, 0>(b) << ", "
    << bg::get<bg::max_corner, 1>(b) << ")" << std::endl;

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

  n = nodes["output"];
  std::string format = n["format"].as<std::string>();
  std::clog << "Lifting all input polygons to 3D..." << std::endl;
  if (format == "CSV-BUILDINGS")
    map3d.threeDfy(false);
  else if (format == "OBJ-BUILDINGS") {
    map3d.threeDfy(false);
    map3d.construct_CDT();
  }
  else {
    map3d.threeDfy(bStitching);
    map3d.construct_CDT();
  }
  std::clog << "done." << std::endl;


  //-- output
  if (n["building_floor"].as<std::string>() == "true")
    map3d.set_building_include_floor(true);
  int z_exaggeration = 0;
  if (n["vertical_exaggeration"])
    z_exaggeration = n["vertical_exaggeration"].as<int>();

  std::ofstream outputfile;
  if (format != "Shapefile")
    outputfile.open(filename);

  if (format == "CityGML") {
    std::clog << "CityGML output" << std::endl;
    map3d.get_citygml(outputfile);
  }
  else if (format == "OBJ") {
    std::clog << "OBJ output" << std::endl;
    map3d.get_obj_per_feature(outputfile, z_exaggeration);
  }
  else if (format == "OBJ-NoID") {
    std::clog << "OBJ (without IDs) output" << std::endl;
    map3d.get_obj_per_class(outputfile, z_exaggeration);
  }
  else if (format == "CSV-BUILDINGS") {
    std::clog << "CSV output (only of the buildings)" << std::endl;
    map3d.get_csv_buildings(outputfile);
  }
  else if (format == "Shapefile") {
    std::clog << "Shapefile output" << std::endl;
    if (map3d.get_shapefile(filename)) {
      std::clog << "Shapefile written" << std::endl;
    }
    else
    {
      std::cerr << "Writing shapefile failed" << std::endl;
      return 0;
    }
  }
  outputfile.close();

  //-- bye-bye
  auto duration = boost::chrono::high_resolution_clock::now() - startTime;
  std::clog << "Successfully terminated in " 
    << boost::chrono::duration_cast<boost::chrono::seconds>(duration) << " || "
    << boost::chrono::duration_cast<boost::chrono::hours>(duration).count() << ":" 
    << boost::chrono::duration_cast<boost::chrono::minutes>(duration).count() << ":" 
    << boost::chrono::duration_cast<boost::chrono::seconds>(duration).count() << std::endl;
  return 1;
}

void print_license() {
  std::string thelicense =
    "\n3dfier: takes 2D GIS datasets and '3dfies' to create 3D city models.\n\n"
    "Copyright (C) 2015-2016  3D geoinformation research group, TU Delft\n\n"
    "3dfier is free software: you can redistribute it and/or modify\n"
    "it under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation, either version 3 of the License, or\n"
    "(at your option) any later version.\n\n"
    "3dfier is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n\n"
    "A copy of the GNU General Public License is available at\n"
    "<http://www.gnu.org/licenses/> or\n"
    "<https://github.com/tudelft3d/3dfier/blob/master/LICENSE\n\n"
    "For any information or further details about the use of 3dfier, contact:\n"
    "Hugo Ledoux \n"
    "<h.ledoux@tudelft.nl>\n"
    "Faculty of Architecture & the Built Environment\n"
    "Delft University of Technology\n"
    "Julianalaan 134, Delft 2628BL, the Netherlands\n";
  std::clog << thelicense << std::endl;
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
      if ((s.substr(0, s.find_first_of("-")) != "percentile") ||
        (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false)) {
        wentgood = false;
        std::cerr << "\tOption 'Building.height_roof' invalid; must be 'percentile-XX'." << std::endl;
      }
    }
    if (n["Building"]["height_ground"]) {
      std::string s = n["Building"]["height_ground"].as<std::string>();
      if ((s.substr(0, s.find_first_of("-")) != "percentile") ||
        (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false)) {
        wentgood = false;
        std::cerr << "\tOption 'Building.height_ground' invalid; must be 'percentile-XX'." << std::endl;
      }
    }
    if (n["Building"]["lod"]) {
      if (is_string_integer(n["Building"]["lod"].as<std::string>(), 0, 1) == false) {
        wentgood = false;
        std::cerr << "\tOption 'Building.lod' invalid; must be an integer between 0 and 1." << std::endl;
      }
    }
    if (n["Building"]["triangulate"]) {
      std::string s = n["Building"]["triangulate"].as<std::string>();
      if ((s != "true") && (s != "false")) {
        wentgood = false;
        std::cerr << "\tOption 'Building.triangulate' invalid; must be 'true' or 'false'." << std::endl;
      }
    }
  }
  if (n["Water"]) {
    if (n["Water"]["height"]) {
      std::string s = n["Water"]["height"].as<std::string>();
      if ((s.substr(0, s.find_first_of("-")) != "percentile") ||
        (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false)) {
        wentgood = false;
        std::cerr << "\tOption 'Water.height' invalid; must be 'percentile-XX'." << std::endl;
      }
    }
  }
  if (n["Road"]) {
    if (n["Road"]["height"]) {
      std::string s = n["Road"]["height"].as<std::string>();
      if ((s.substr(0, s.find_first_of("-")) != "percentile") ||
        (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false)) {
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
    catch (boost::bad_lexical_cast& e) {
      wentgood = false;
      std::cerr << "\tOption 'options.radius_vertex_elevation' invalid." << std::endl;
    }
  }
  if (n["threshold_jump_edges"]) {
    try {
      boost::lexical_cast<float>(n["threshold_jump_edges"].as<std::string>());
    }
    catch (boost::bad_lexical_cast& e) {
      wentgood = false;
      std::cerr << "\tOption 'options.threshold_jump_edges' invalid." << std::endl;
    }
  }
  //-- 5. output
  n = nodes["output"];
  std::string format = n["format"].as<std::string>();
  if ((format != "OBJ") &&
    (format != "OBJ-NoID") &&
    (format != "CityGML") &&
    (format != "OBJ-BUILDINGS") &&
    (format != "CSV-BUILDINGS") &&
    (format != "Shapefile")) {
    wentgood = false;
    std::cerr << "\tOption 'output.format' invalid (OBJ | OBJ-NoID | CityGML | CSV-BUILDINGS | Shapefile)" << std::endl;
  }
  //-- Shapefile type filename check
  if (format == "Shapefile" && !n["filename"]) {
    wentgood = false;
    std::cerr << "\tOption 'output.format' Shapefile needs an output.filename" << std::endl;
  }
  return wentgood;
}
