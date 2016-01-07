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


//-- TODO: add GDAL2.0 support
//-- TODO: filter the noise in the lidar points inside TIN?
//-- TODO : how to make roads horizontal "in the length"? or do we need to?

//-----------------------------------------------------------------------------

#include "definitions.h"
#include "io.h"
#include "TopoFeature.h"
#include "Map3d.h"
#include "yaml-cpp/yaml.h"

bool validate_yaml(YAML::Node& nodes, std::set<std::string>& allowedFeatures); 



int main(int argc, const char * argv[]) {
  
//-- reading the config file
  if (argc != 2) {
    std::cerr << "ERROR: the config file (*.yml) is not defined." << std::endl;
    return 0;
  }
  std::clog << "Reading config file: " << argv[1] << std::endl;

//-- allowed feature classes
  std::set<std::string> allowedFeatures;
  allowedFeatures.insert("Building");
  allowedFeatures.insert("Water");
  allowedFeatures.insert("Terrain");
  allowedFeatures.insert("Road");
  allowedFeatures.insert("Vegetation");
  allowedFeatures.insert("Bridge/Overpass");

  
  Map3d map3d;
  YAML::Node nodes = YAML::LoadFile(argv[1]);

//-- validate the YAML file right now, nicer for the user
  validate_yaml(nodes, allowedFeatures);
  
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
    if ((*it)["lifting_per_layer"]) {
      YAML::Node tmp = (*it)["lifting_per_layer"];
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
    std::cerr << "ERROR: Something went bad while reading input polygons. Aborting." << std::endl;
    return 0;
  }

  std::clog << "\nTotal # of polygons: " << map3d.get_num_polygons() << std::endl;
  
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
      map3d.add_las_file(it2->as<std::string>(), lasomits, (*it)["thinning"].as<int>());
    }
  }

  if (bElevData == false) {
    std::cerr << "ERROR: No elevation dataset given, cannot 3dfy the dataset. Aborting." << std::endl;
    return 0;
  }

  n = nodes["output"];
  std::clog << "Lifting all input polygons to 3D..." << std::endl;
  if (n["format"].as<std::string>() == "CSV-BUILDINGS")
    map3d.threeDfy(false);
  else
    map3d.threeDfy();
  std::clog << "done." << std::endl;
  
  
  //-- output
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
  else {
    std::cerr << "ERROR: Output format " << n["format"].as<std::string>() << " not recognised. Outputting OBJ." << std::endl;
    std::cout << map3d.get_obj() << std::endl;
  }

  //-- bye-bye
  std::clog << "Successfully terminated." << std::endl;
  return 1;
}


bool validate_yaml(YAML::Node& nodes, std::set<std::string>& allowedFeatures) {
  YAML::Node n = nodes["input_polygons"];
  for (auto it = n.begin(); it != n.end(); ++it) {
    if ((*it)["lifting_per_layer"]) {
      YAML::Node tmp = (*it)["lifting_per_layer"];
      std::vector<std::pair<std::string, std::string> > layers;
      // for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
      //   std::pair<std::string, std::string> onepair( (it2->first).as<std::string>(), (it2->second).as<std::string>() );
      //   layers.push_back(onepair);
      // }
      // tmp = (*it)["datasets"];
      // for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
      //   wentgood = map3d.add_polygons_file(it2->as<std::string>(),
      //                                      (*it)["uniqueid"].as<std::string>(),
      //                                      layers);
      // }
    }
    else if ((*it)["lifting"]) {
      YAML::Node tmp = (*it)["datasets"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
        if (allowedFeatures.count((*it)["lifting"].as<std::string>()) == 1) {
          std::cout << "allowed" << std::endl;
          // wentgood = map3d.add_polygons_file(it2->as<std::string>(),
                                             // (*it)["uniqueid"].as<std::string>(),
                                             // (*it)["lifting"].as<std::string>());
        }
        else {
          std::clog << "ERROR: class '" << (*it)["lifting"].as<std::string>() << "' not recognised." << std::endl;
          // wentgood = false;
        }
      }
    }
  }  
  return true;
}


