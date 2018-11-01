/*
  3dfier: takes 2D GIS datasets and "3dfies" to create 3D city models.

  Copyright (C) 2015-2018  3D geoinformation research group, TU Delft

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

#include "yaml-cpp/yaml.h"
#include "definitions.h"
#include "io.h"
#include "TopoFeature.h"
#include "Map3d.h"
#include "boost/locale.hpp"
#include "boost/chrono.hpp"
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

std::string VERSION = "1.0.3";

bool validate_yaml(const char* arg, std::set<std::string>& allowedFeatures);
int main(int argc, const char * argv[]);
std::string print_license();
void print_duration(std::string message, boost::chrono::time_point<boost::chrono::steady_clock> startTime);

int main(int argc, const char * argv[]) {
  auto startTime = boost::chrono::high_resolution_clock::now();
  boost::locale::generator gen;
  std::locale loc = gen("en_US.UTF-8");
  std::locale::global(loc);
  std::clog.imbue(loc);
  std::cout.imbue(loc);

  std::string licensewarning =
    "3dfier Copyright (C) 2015-2018  3D geoinformation research group, TU Delft\n"
    "This program comes with ABSOLUTELY NO WARRANTY.\n"
    "This is free software, and you are welcome to redistribute it\n"
    "under certain conditions; for details run 3dfier with the '--license' option.\n";

  std::map<std::string, std::string> outputs;
  outputs["OBJ"] = "";
  outputs["OBJ-NoID"] = "";
  outputs["CityGML"] = "";
  outputs["CityGML-Multifile"] = "";
  outputs["CityGML-IMGeo"] = "";
  outputs["CityGML-IMGeo-Multifile"] = "";
  outputs["CityJSON"] = "";
  outputs["CSV-BUILDINGS"] = "";
  outputs["CSV-BUILDINGS-MULTIPLE"] = "";
  outputs["CSV-BUILDINGS-ALL-Z"] = "";
  outputs["Shapefile"] = "";
  outputs["Shapefile-Multifile"] = "";
  outputs["PostGIS"] = "";
  outputs["PostGIS-Multi"] = "";
  outputs["PostGIS-PDOK"] = "";
  outputs["PostGIS-PDOK-CityGML"] = "";
  outputs["GDAL"] = "";
  std::string f_yaml;
  try {
    namespace po = boost::program_options;
    po::options_description pomain("Allowed options");
    pomain.add_options()
      ("help", "View all options")
      ("version", "View version")
      ("license", "View license")
      ("OBJ", po::value<std::string>(&outputs["OBJ"]), "Output ")
      ("OBJ-NoID", po::value<std::string>(&outputs["OBJ-NoID"]), "Output ")
      ("CityGML", po::value<std::string>(&outputs["CityGML"]), "Output ")
      ("CityGML-Multifile", po::value<std::string>(&outputs["CityGML-Multifile"]), "Output ")
      ("CityGML-IMGeo", po::value<std::string>(&outputs["CityGML-IMGeo"]), "Output ")
      ("CityGML-IMGeo-Multifile", po::value<std::string>(&outputs["CityGML-IMGeo-Multifile"]), "Output ")
      ("CityJSON", po::value<std::string>(&outputs["CityJSON"]), "Output ")
      ("CSV-BUILDINGS", po::value<std::string>(&outputs["CSV-BUILDINGS"]), "Output ")
      ("CSV-BUILDINGS-MULTIPLE", po::value<std::string>(&outputs["CSV-BUILDINGS-MULTIPLE"]), "Output ")
      ("CSV-BUILDINGS-ALL-Z", po::value<std::string>(&outputs["CSV-BUILDINGS-ALL-Z"]), "Output ")
      ("Shapefile", po::value<std::string>(&outputs["Shapefile"]), "Output ")
      ("Shapefile-Multifile", po::value<std::string>(&outputs["Shapefile-Multifile"]), "Output ")
      ("PostGIS", po::value<std::string>(&outputs["PostGIS"]), "Output ")
      ("PostGIS-Multi", po::value<std::string>(&outputs["PostGIS-Multi"]), "Output ")
      ("PostGIS-PDOK", po::value<std::string>(&outputs["PostGIS-PDOK"]), "Output ")
      ("PostGIS-PDOK-CityGML", po::value<std::string>(&outputs["PostGIS-PDOK-CityGML"]), "Output ")
      ("GDAL", po::value<std::string>(&outputs["GDAL"]), "Output ")
      ;
    po::options_description pohidden("Hidden options");
    pohidden.add_options()
      ("yaml", po::value<std::string>(&f_yaml), "Input config YAML file")
      ;
    po::positional_options_description popos;
    popos.add("yaml", -1);

    po::options_description poall;
    poall.add(pomain).add(pohidden);
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
      options(poall).positional(popos).run(), vm);
    po::notify(vm);

    std::clog << licensewarning << std::endl;

    if (vm.count("help")) {
      std::cout << "Usage: 3dfier config.yml --OBJ myoutput.obj" << std::endl;
      std::cout << "       3dfier config.yml --CityGML myoutput.gml" << std::endl;
      std::cout << pomain << std::endl;
      return EXIT_SUCCESS;
    }
    if (vm.count("license")) {
      std::cout << print_license() << std::endl;
      return EXIT_SUCCESS;
    }
    if (vm.count("version")) {
      std::cout << "3dfier " << VERSION << std::endl;
      std::cout << liblas::GetFullVersion() << std::endl;
      std::cout << "GDAL " << GDALVersionInfo("--version") << std::endl;
      //-- TODO : put here the date and/or the git-commit?
      return EXIT_SUCCESS;
    }
    if (vm.count("yaml") == 0) {
      std::cerr << "ERROR: one YAML config file must be specified." << std::endl;
      std::cout << std::endl << pomain << std::endl;
      return EXIT_FAILURE;
    }
    else {
      boost::filesystem::path yp(f_yaml);
      if (boost::filesystem::exists(yp) == false) {
        std::cerr << "ERROR: YAML file " << f_yaml << " doesn't exist." << std::endl;
        return EXIT_FAILURE;
      }
      if (yp.extension() != ".yml") {
        std::cerr << "ERROR: config file " << f_yaml << " if not *.yml" << std::endl;
        return EXIT_FAILURE;
      }
    }
    for (auto& output : vm) {
      if ((output.first != "yaml") && (output.first.find("PostGIS") == std::string::npos)) {
        //-- check paths of the output file
        boost::filesystem::path p(outputs[output.first]);
        try {
          boost::filesystem::path pcan = canonical(p.parent_path(), boost::filesystem::current_path());
        }
        catch (boost::filesystem::filesystem_error &e) {
          std::cerr << "ERROR: " << e.what() << ". Abort." << std::endl;
          return EXIT_FAILURE;
        }
      }
    }
  }
  catch (std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return EXIT_FAILURE;
  }

  //-- allowed feature classes
  std::set<std::string> allowedFeatures{ "Building", "Water", "Terrain", "Road", "Forest", "Separation", "Bridge/Overpass" };

  //-- validate the YAML file right now, nicer for the user
   if (validate_yaml(argv[1], allowedFeatures) == false) {
     std::cerr << "ERROR: config file (*.yml) is not valid. Aborting.\n";
     return EXIT_FAILURE;
   }
   std::clog << "Config file is valid.\n";

  Map3d map3d;
  YAML::Node nodes = YAML::LoadFile(f_yaml);
  //-- store the lifting options in the Map3d
  if (nodes["lifting_options"]) {
    YAML::Node n = nodes["lifting_options"];
    if (n["Building"]) {
      if (n["Building"]["roof"]) {
        if (n["Building"]["roof"]["height"]) {
          std::string height = n["Building"]["roof"]["height"].as<std::string>();
          map3d.set_building_heightref_roof(std::stof(height.substr(height.find_first_of("-") + 1)) / 100);
        }
        YAML::Node tmp = n["Building"]["roof"]["use_LAS_classes"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
          map3d.add_allowed_las_class(LAS_BUILDING_ROOF, it2->as<int>());
        if (n["Building"]["roof"]["use_LAS_classes_within"]) {
          tmp = n["Building"]["roof"]["use_LAS_classes_within"];
          for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
            map3d.add_allowed_las_class_within(LAS_BUILDING_ROOF, it2->as<int>());
        }
      }
      if (n["Building"]["ground"]) {
        if (n["Building"]["ground"]["height"]) {
          std::string height = n["Building"]["ground"]["height"].as<std::string>();
          map3d.set_building_heightref_ground(std::stof(height.substr(height.find_first_of("-") + 1)) / 100);
        }
        YAML::Node tmp = n["Building"]["ground"]["use_LAS_classes"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
          map3d.add_allowed_las_class(LAS_BUILDING_GROUND, it2->as<int>());
        if (n["Building"]["ground"]["use_LAS_classes_within"]) {
          tmp = n["Building"]["ground"]["use_LAS_classes_within"];
          for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
            map3d.add_allowed_las_class_within(LAS_BUILDING_GROUND, it2->as<int>());
        }
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
      if (n["Building"]["floor"]) {
        if (n["Building"]["floor"].as<std::string>() == "true")
          map3d.set_building_include_floor(true);
        else
          map3d.set_building_include_floor(false);
      }
      if (n["Building"]["inner_walls"]) {
        if (n["Building"]["inner_walls"].as<std::string>() == "true")
          map3d.set_building_inner_walls(true);
        else
          map3d.set_building_inner_walls(false);
      }
    }
    if (n["Terrain"]) {
      if (n["Terrain"]["simplification"])
        map3d.set_terrain_simplification(n["Terrain"]["simplification"].as<int>());
      if (n["Terrain"]["simplification_tinsimp"] != 0)
        map3d.set_terrain_simplification_tinsimp(n["Terrain"]["simplification_tinsimp"].as<double>());
      if (n["Terrain"]["innerbuffer"])
        map3d.set_terrain_innerbuffer(n["Terrain"]["innerbuffer"].as<float>());
      YAML::Node tmp = n["Terrain"]["use_LAS_classes"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
        map3d.add_allowed_las_class(LAS_TERRAIN, it2->as<int>());
      if (n["Terrain"]["use_LAS_classes_within"]) {
        tmp = n["Terrain"]["use_LAS_classes_within"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
          map3d.add_allowed_las_class_within(LAS_TERRAIN, it2->as<int>());
      }
    }
    if (n["Forest"]) {
      if (n["Forest"]["simplification"])
        map3d.set_forest_simplification(n["Forest"]["simplification"].as<int>());
      if (n["Forest"]["simplification_tinsimp"] != 0)
        map3d.set_forest_simplification_tinsimp(n["Forest"]["simplification_tinsimp"].as<double>());
      if (n["Forest"]["innerbuffer"])
        map3d.set_forest_innerbuffer(n["Forest"]["innerbuffer"].as<float>());
      YAML::Node tmp = n["Forest"]["use_LAS_classes"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
        map3d.add_allowed_las_class(LAS_FOREST, it2->as<int>());
      if (n["Forest"]["use_LAS_classes_within"]) {
        tmp = n["Forest"]["use_LAS_classes_within"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
          map3d.add_allowed_las_class_within(LAS_FOREST, it2->as<int>());
      }
    }
    if (n["Water"]) {
      if (n["Water"]["height"]) {
        std::string height = n["Water"]["height"].as<std::string>();
        map3d.set_water_heightref(std::stof(height.substr(height.find_first_of("-") + 1)) / 100);
      }
      YAML::Node tmp = n["Water"]["use_LAS_classes"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
        map3d.add_allowed_las_class(LAS_WATER, it2->as<int>());
      if (n["Water"]["use_LAS_classes_within"]) {
        tmp = n["Water"]["use_LAS_classes_within"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
          map3d.add_allowed_las_class_within(LAS_WATER, it2->as<int>());
      }
    }
    if (n["Road"]) {
      if (n["Road"]["height"]) {
        std::string height = n["Road"]["height"].as<std::string>();
        map3d.set_road_heightref(std::stof(height.substr(height.find_first_of("-") + 1)) / 100);
      }
      if (n["Road"]["filter_outliers"]) {
        if (n["Road"]["filter_outliers"].as<std::string>() == "true")
          map3d.set_road_filter_outliers(true);
        else
          map3d.set_road_filter_outliers(false);
      }
      if (n["Road"]["flatten"]) {
        if (n["Road"]["flatten"].as<std::string>() == "true")
          map3d.set_road_flatten(true);
        else
          map3d.set_road_flatten(false);
      }
      YAML::Node tmp = n["Road"]["use_LAS_classes"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
        map3d.add_allowed_las_class(LAS_ROAD, it2->as<int>());
      if (n["Road"]["use_LAS_classes_within"]) {
        tmp = n["Road"]["use_LAS_classes_within"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
          map3d.add_allowed_las_class_within(LAS_ROAD, it2->as<int>());
      }
    }
    if (n["Separation"]) {
      if (n["Separation"]["height"]) {
        std::string height = n["Separation"]["height"].as<std::string>();
        map3d.set_separation_heightref(std::stof(height.substr(height.find_first_of("-") + 1)) / 100);
      }
      YAML::Node tmp = n["Separation"]["use_LAS_classes"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
        map3d.add_allowed_las_class(LAS_SEPARATION, it2->as<int>());
      if (n["Separation"]["use_LAS_classes_within"]) {
        tmp = n["Separation"]["use_LAS_classes_within"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
          map3d.add_allowed_las_class_within(LAS_SEPARATION, it2->as<int>());
      }
    }
    if (n["Bridge/Overpass"]) {
      if (n["Bridge/Overpass"]["height"]) {
        std::string height = n["Bridge/Overpass"]["height"].as<std::string>();
        map3d.set_bridge_heightref(std::stof(height.substr(height.find_first_of("-") + 1)) / 100);
      }
      if (n["Bridge/Overpass"]["flatten"]) {
        if (n["Bridge/Overpass"]["flatten"].as<std::string>() == "true")
          map3d.set_bridge_flatten(true);
        else
          map3d.set_bridge_flatten(false);
      }
      YAML::Node tmp = n["Bridge/Overpass"]["use_LAS_classes"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
        map3d.add_allowed_las_class(LAS_BRIDGE, it2->as<int>());
      if (n["Bridge/Overpass"]["use_LAS_classes_within"]) {
        tmp = n["Bridge/Overpass"]["use_LAS_classes_within"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
          map3d.add_allowed_las_class_within(LAS_BRIDGE, it2->as<int>());
      }
    }
  }

  bool bStitching = true;
  //-- set al general options
  if (nodes["options"]) {
    YAML::Node n = nodes["options"];
    if (n["radius_vertex_elevation"])
      map3d.set_radius_vertex_elevation(n["radius_vertex_elevation"].as<float>());
    if (n["building_radius_vertex_elevation"])
      map3d.set_building_radius_vertex_elevation(n["building_radius_vertex_elevation"].as<float>());
    if (n["threshold_jump_edges"])
      map3d.set_threshold_jump_edges(n["threshold_jump_edges"].as<float>());
    if (n["stitching"] && n["stitching"].as<std::string>() == "false")
      bStitching = false;

    if (n["extent"]) {
      std::vector<std::string> extent_split = stringsplit(n["extent"].as<std::string>(), ',');
      double xmin, xmax, ymin, ymax;
      bool wentgood = true;
      try {
        xmin = boost::lexical_cast<double>(extent_split[0]);
        ymin = boost::lexical_cast<double>(extent_split[1]);
        xmax = boost::lexical_cast<double>(extent_split[2]);
        ymax = boost::lexical_cast<double>(extent_split[3]);
      }
      catch (boost::bad_lexical_cast& e) {
        wentgood = false;
      }

      if (!wentgood || xmin > xmax || ymin > ymax || boost::geometry::area(Box2(Point2(xmin, ymin), Point2(xmax, ymax))) <= 0.0) {
        std::cerr << "ERROR: The supplied extent is not valid: (" << n["extent"].as<std::string>() << "), using all polygons\n";
      }
      else {
        std::clog << "Using extent for polygons: (" << n["extent"].as<std::string>() << ")\n";
        map3d.set_requested_extent(xmin, ymin, xmax, ymax);
      }
    }
  }

  //-- read polygon data configuration
  std::vector<PolygonFile> polygonFiles;
  if (nodes["input_polygons"]) {
    YAML::Node n = nodes["input_polygons"];
    for (auto it = n.begin(); it != n.end(); ++it) {
      // Get the correct uniqueid attribute
      std::string uniqueid = "fid";
      if ((*it)["uniqueid"]) {
        uniqueid = (*it)["uniqueid"].as<std::string>();
      }
      // Get the correct height attribute
      std::string heightfield = "";
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
          polygonFiles.push_back(file);
        }
        else if ((*it)["lifting_per_layer"]) {
          YAML::Node layers = (*it)["lifting_per_layer"];
          for (auto it3 = layers.begin(); it3 != layers.end(); ++it3) {
            file.layers.emplace_back(it3->first.as<std::string>(), it3->second.as<std::string>());
          }
          polygonFiles.push_back(file);
        }
      }
    }
  }

  //-- check if all polygon files exist
  bool bPolyData = false;
#if GDAL_VERSION_MAJOR < 2
  if (OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount() == 0)
    OGRRegisterAll();
#else
  if (GDALGetDriverCount() == 0)
    GDALAllRegister();
#endif

  for (auto file = polygonFiles.begin(); file != polygonFiles.end(); ++file) {
#if GDAL_VERSION_MAJOR < 2
    OGRDataSource *dataSource = OGRSFDriverRegistrar::Open(file->filename.c_str(), false);
#else
    GDALDataset *dataSource = (GDALDataset*)GDALOpenEx(file->filename.c_str(), GDAL_OF_READONLY | GDAL_OF_VECTOR, NULL, NULL, NULL);
#endif
    if (dataSource != NULL) {
      bPolyData = true;
    }
#if GDAL_VERSION_MAJOR < 2
    OGRDataSource::DestroyDataSource(dataSource);
#else
    GDALClose(dataSource);
#endif
    if (bPolyData == false) {
      std::string logstring = "Reading input dataset: " + file->filename;
      if (strncmp(file->filename.c_str(), "PG:", strlen("PG:")) == 0) {
        logstring = "Opening PostgreSQL database connection.";
      }
      std::cerr << "\tERROR: " << logstring << std::endl;
      return EXIT_FAILURE;
    }
  }

  //-- read elevation data configuration
  std::vector<PointFile> elevationFiles;
  //-- add elevation datasets
  if (nodes["input_elevation"]) {
    YAML::Node n = nodes["input_elevation"];
    for (auto it = n.begin(); it != n.end(); ++it) {
      YAML::Node tmp = (*it)["omit_LAS_classes"];
      std::vector<int> lasomits;
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2)
        lasomits.push_back(it2->as<int>());
      tmp = (*it)["datasets"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
        int thinning = 1;
        if ((*it)["thinning"]) {
          thinning = (*it)["thinning"].as<int>();
          if (thinning == 0) {
            thinning = 1;
          }
        }

        //-- iterate over all files in directory
        boost::filesystem::path path(it2->as<std::string>());
        boost::filesystem::path rootPath = path.parent_path();
        if (path.stem() == "*") {
          if (!boost::filesystem::exists(rootPath) || !boost::filesystem::is_directory(rootPath)) {
            std::cerr << "ERROR: " << rootPath << "is not a directory.\n";
            return EXIT_FAILURE;
          }
          else {
            boost::filesystem::recursive_directory_iterator it_end;
            for (boost::filesystem::recursive_directory_iterator it(rootPath); it != it_end; ++it) {
              if (boost::filesystem::is_regular_file(*it) && it->path().extension() == path.extension()) {
                PointFile pointFile;
                pointFile.filename = it->path().string();
                pointFile.lasomits = lasomits;
                pointFile.thinning = thinning;
                elevationFiles.push_back(pointFile);
              }
            }
          }
        }
        else {
          PointFile pointFile;
          pointFile.filename = path.string();
          pointFile.lasomits = lasomits;
          pointFile.thinning = thinning;
          elevationFiles.push_back(pointFile);
        }
      }
    }
  }
  //-- check if all elevation files exist
  for (auto file : elevationFiles) {
    std::ifstream f(file.filename);
    if (!f.good()) {
      std::cerr << "ERROR: cannot open file " << file.filename << std::endl;
      return EXIT_FAILURE;
    }
  }

  //-- add the polygons to the map3d
  if (bPolyData) {
    bPolyData = map3d.add_polygons_files(polygonFiles);
  }
  if (!bPolyData) {
    std::cerr << "ERROR: Missing polygon data, cannot 3dfy the dataset. Aborting.\n";
    return EXIT_FAILURE;
  }
  std::clog << "\nTotal # of polygons: " << boost::locale::as::number << map3d.get_num_polygons() << std::endl;

  map3d.save_building_variables();
  //-- spatially index the polygons
  map3d.construct_rtree();

  //-- print bbox from _rtree
  Box2 b = map3d.get_bbox();
  std::clog << std::setprecision(3) << std::fixed;
  std::clog << "Spatial extent: ("
    << bg::get<bg::min_corner, 0>(b) << ", "
    << bg::get<bg::min_corner, 1>(b) << ") ("
    << bg::get<bg::max_corner, 0>(b) << ", "
    << bg::get<bg::max_corner, 1>(b) << ")\n";

  //-- add the elevation data to the map3d
  auto startPoints = boost::chrono::high_resolution_clock::now();
  for (auto file : elevationFiles) {
    bool added = map3d.add_las_file(file);
    if (!added) {
      std::cerr << "ERROR: corrupt file " << file.filename << std::endl;
      return EXIT_FAILURE;
    }
  }
  print_duration("All points read in %lld seconds || %02d:%02d:%02d\n", startPoints);
  map3d.cleanup_grids();

  std::clog << "3dfying all input polygons...\n";
  bool threedfy = true;
  bool cdt = true;
  int outputcount = 0;
  for (auto& each : outputs) {
    if (each.second != "") {
      outputcount++;
    }
  }
  if (outputcount == 1) {
    if (outputs["CSV-BUILDINGS"] != "") {
      threedfy = false;
      cdt = false;
      std::clog << "CSV-BUILDINGS: no 3D reconstruction" << std::endl;
    }
    else if (outputs["CSV-BUILDINGS-MULTIPLE"] != "") {
      threedfy = false;
      cdt = false;
      std::clog << "CSV-BUILDINGS-MULTIPLE: no 3D reconstruction" << std::endl;
    }
    else if (outputs["CSV-BUILDINGS-ALL-Z"] != "") {
      threedfy = false;
      cdt = false;
      std::clog << "CSV-BUILDINGS-ALL-Z: no 3D reconstruction" << std::endl;
    }
    else if (outputs["OBJ-NoID"] != "") {
      // for OBJ-NoID only lift objects, skip stitching
      bStitching = false;
    }
  }
  if (threedfy) {
    auto startThreeDfy = boost::chrono::high_resolution_clock::now();
    map3d.threeDfy(bStitching);
    print_duration("Lifting, stitching and vertical walls done in %lld seconds || %02d:%02d:%02d\n", startThreeDfy);
  }
  if (cdt) {
    auto startCDT = boost::chrono::high_resolution_clock::now();
    if (!map3d.construct_CDT()) {
      return EXIT_FAILURE;
    }
    print_duration("CDT created in %lld seconds || %02d:%02d:%02d\n", startCDT);
  }
  std::clog << "...3dfying done.\n";
  map3d.cleanup_elevations();

  //-- iterate over all output
  for (auto& output : outputs) {
    auto startFileWriting = boost::chrono::high_resolution_clock::now();
    std::string format = output.first;
    if (output.second == "")
      continue;

    bool fileWritten = true;
    std::wofstream of;
    std::string ofname = output.second;
    if (format != "CityGML-Multifile" && format != "CityGML-IMGeo-Multifile" && format != "CityJSON" &&
      format != "Shapefile" && format != "Shapefile-Multifile" &&
      format != "PostGIS" && format != "PostGIS-Multi" && format != "PostGIS-PDOK" && format != "PostGIS-PDOK-CityGML" &&
      format != "GDAL") {
      of.open(ofname);
    }
    if (format == "CityGML") {
      std::clog << "CityGML output: " << ofname << std::endl;
      map3d.get_citygml(of);
    }
    else if (format == "CityGML-Multifile") {
      std::clog << "CityGML multiple file output: " << ofname << std::endl;
      map3d.get_citygml_multifile(ofname);
    }
    else if (format == "CityGML-IMGeo") {
      std::clog << "IMGeo (CityGML ADE) output: " << ofname << std::endl;
      map3d.get_citygml_imgeo(of);
    }
    else if (format == "CityGML-IMGeo-Multifile") {
      std::clog << "IMGeo (CityGML ADE) multiple file output: " << ofname << std::endl;
      map3d.get_citygml_imgeo_multifile(ofname);
    }
    else if (format == "CityJSON") {
      std::clog << "CityJSON output: " << ofname << std::endl;
      fileWritten = map3d.get_cityjson(ofname);
    }
    else if (format == "OBJ") {
      std::clog << "OBJ output: " << ofname << std::endl;
      map3d.get_obj_per_feature(of);
    }
    else if (format == "OBJ-NoID") {
      std::clog << "OBJ (without IDs) output: " << ofname << std::endl;
      map3d.get_obj_per_class(of);
    }
    else if (format == "CSV-BUILDINGS") {
      std::clog << "CSV output (only of the buildings): " << ofname << std::endl;
      map3d.get_csv_buildings(of);
    }
    else if (format == "CSV-BUILDINGS-MULTIPLE") {
      std::clog << "CSV output with multiple heights (only of the buildings): " << ofname << std::endl;
      map3d.get_csv_buildings_multiple_heights(of);
    }
    else if (format == "CSV-BUILDINGS-ALL-Z") {
      std::clog << "CSV output with all z values (only of the buildings): " << ofname << std::endl;
      map3d.get_csv_buildings_all_elevation_points(of);
    }
    else if (format == "Shapefile") {
      std::clog << "Shapefile output: " << ofname << std::endl;
      fileWritten = map3d.get_gdal_output(ofname, "ESRI Shapefile", false);
    }
    else if (format == "Shapefile-Multifile") {
      std::clog << "Shapefile multiple file output: " << ofname << std::endl;
      fileWritten = map3d.get_gdal_output(ofname, "ESRI Shapefile", true);
    }
    else if (format == "PostGIS") {
      std::clog << "PostGIS output\n";
      fileWritten = map3d.get_gdal_output(ofname, "PostgreSQL", false);
    }
    else if (format == "PostGIS-Multi") {
      std::clog << "PostGIS multiple table output\n";
      fileWritten = map3d.get_gdal_output(ofname, "PostgreSQL", true);
    }
    else if (format == "PostGIS-PDOK") {
      std::clog << "PostGIS with IMGeo GML string output\n";
      fileWritten = map3d.get_pdok_output(ofname);
    }
    else if (format == "PostGIS-PDOK-CityGML") {
      std::clog << "PostGIS with CityGML string output\n";
      fileWritten = map3d.get_pdok_citygml_output(ofname);
    }
    else if (format == "GDAL") { //-- TODO: what is this? a path? how to use?
      if (nodes["output"] && nodes["output"]["gdal_driver"]) {
        std::string driver = nodes["output"]["gdal_driver"].as<std::string>();
        std::clog << "GDAL output using driver '" + driver + "'\n";
        fileWritten = map3d.get_gdal_output(ofname, driver, false);
      }
    }
    of.close();

    if (fileWritten) {
      print_duration("Features written in %d seconds || %02d:%02d:%02d\n", startFileWriting);
    }
    else {
      std::cerr << "ERROR: Writing features failed for " << format << ". Aborting.\n";
      return EXIT_FAILURE;
    }
  }

  //-- bye-bye
  print_duration("Successfully terminated in %d seconds || %02d:%02d:%02d\n", startTime);
  return EXIT_SUCCESS;
}

std::string print_license() {
  std::string thelicense =
    "======================================================================\n"
    "\n3dfier: takes 2D GIS datasets and '3dfies' to create 3D city models.\n\n"
    "Copyright (C) 2015-2018  3D geoinformation research group, TU Delft\n\n"
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
    "For any information or further details about 3dfier, contact:\n"
    "Hugo Ledoux \n"
    "<h.ledoux@tudelft.nl>\n"
    "Faculty of Architecture & the Built Environment\n"
    "Delft University of Technology\n"
    "Julianalaan 134, Delft 2628BL, the Netherlands\n"
    "======================================================================";
  return thelicense;
}

void print_duration(std::string message, boost::chrono::time_point<boost::chrono::steady_clock> startTime) {
  auto duration = boost::chrono::high_resolution_clock::now() - startTime;
  printf(message.c_str(),
    boost::chrono::duration_cast<boost::chrono::seconds>(duration).count(),
    boost::chrono::duration_cast<boost::chrono::hours>(duration).count(),
    boost::chrono::duration_cast<boost::chrono::minutes>(duration).count() % 60,
    (int)boost::chrono::duration_cast<boost::chrono::seconds>(duration).count() % 60
  );
}

bool validate_yaml(const char* arg, std::set<std::string>& allowedFeatures) {
  YAML::Node nodes;
  try {
    nodes = YAML::LoadFile(arg);
  }
  catch (const std::exception&) {
    std::cerr << "ERROR: YAML structure of config is invalid.\n";
    return false;
  }
  bool wentgood = true;
  //-- 1. input polygons classes
  if (nodes["input_polygons"]) {
    YAML::Node n = nodes["input_polygons"];
    for (auto it = n.begin(); it != n.end(); ++it) {
      if ((*it)["lifting_per_layer"]) {
        YAML::Node tmp = (*it)["lifting_per_layer"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
          if (allowedFeatures.count((it2->second).as<std::string>()) == 0) {
            std::cerr << "\tLifting class '" << (it2->second).as<std::string>() << "' unknown.\n";
            wentgood = false;
          }
        }
      }
      else if ((*it)["lifting"]) {
        if ((*it)["lifting"].IsNull()) {
          std::cerr << "Option 'lifting' invalid; supplied empty attribute. \n";
          wentgood = false;
        }
        else if (allowedFeatures.count((*it)["lifting"].as<std::string>()) == 0) {
          std::cerr << "\tLifting class '" << (*it)["lifting"].as<std::string>() << "' unknown.\n";
          wentgood = false;
        }
        if ((*it)["uniqueid"].IsNull()) {
          std::cerr << "Option 'uniqueid' invalid; supplied empty attribute. \n";
          wentgood = false;
        }
        if ((*it)["height_field"].IsNull()) {
          std::cerr << "Option 'height_field' invalid; supplied empty attribute. \n";
          wentgood = false;
        }
      }
    }
  }
  else {
    std::cerr << "Group 'input_polygons' not defined. \n";
    wentgood = false;
  }

  //-- 2. lifting_options
  if (nodes["lifting_options"]) {
    YAML::Node n = nodes["lifting_options"];
    if (n["Building"]) {
      if (n["Building"]["roof"]) {
        if (n["Building"]["roof"]["height"]) {
          std::string s = n["Building"]["roof"]["height"].as<std::string>();
          if ((s.substr(0, s.find_first_of("-")) != "percentile") ||
            (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false)) {
            wentgood = false;
            std::cerr << "\tOption 'Building.roof.height' invalid; must be 'percentile-XX'.\n";
          }
        }
        if (n["Building"]["roof"]["use_LAS_classes"]) {
          YAML::Node tmp = n["Building"]["roof"]["use_LAS_classes"];
          for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
            if (is_string_integer(it2->as<std::string>()) == false) {
              wentgood = false;
              std::cerr << "\tOption 'Building.roof.use_LAS_classes' invalid; must be an integer.\n";
            }
          }
        }
        if (n["Building"]["roof"]["use_LAS_classes_within"]) {
          YAML::Node tmp = n["Building"]["roof"]["use_LAS_classes_within"];
          for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
            if (is_string_integer(it2->as<std::string>()) == false) {
              wentgood = false;
              std::cerr << "\tOption 'Building.roof.use_LAS_classes_within' invalid; must be an integer.\n";
            }
          }
        }
      }
      if (n["Building"]["ground"]) {
        if (n["Building"]["ground"]["height"]) {
          std::string s = n["Building"]["ground"]["height"].as<std::string>();
          if ((s.substr(0, s.find_first_of("-")) != "percentile") ||
            (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false)) {
            wentgood = false;
            std::cerr << "\tOption 'Building.ground.height' invalid; must be 'percentile-XX'.\n";
          }
        }
        if (n["Building"]["ground"]["use_LAS_classes"]) {
          YAML::Node tmp = n["Building"]["ground"]["use_LAS_classes"];
          for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
            if (is_string_integer(it2->as<std::string>()) == false) {
              wentgood = false;
              std::cerr << "\tOption 'Building.ground.use_LAS_classes' invalid; must be an integer.\n";
            }
          }
        }
        if (n["Building"]["ground"]["use_LAS_classes_within"]) {
          YAML::Node tmp = n["Building"]["ground"]["use_LAS_classes_within"];
          for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
            if (is_string_integer(it2->as<std::string>()) == false) {
              wentgood = false;
              std::cerr << "\tOption 'Building.ground.use_LAS_classes_within' invalid; must be an integer.\n";
            }
          }
        }
      }
      if (n["Building"]["lod"]) {
        if (is_string_integer(n["Building"]["lod"].as<std::string>(), 0, 1) == false) {
          wentgood = false;
          std::cerr << "\tOption 'Building.lod' invalid; must be an integer between 0 and 1.\n";
        }
      }
      if (n["Building"]["triangulate"]) {
        std::string s = n["Building"]["triangulate"].as<std::string>();
        if ((s != "true") && (s != "false")) {
          wentgood = false;
          std::cerr << "\tOption 'Building.triangulate' invalid; must be 'true' or 'false'.\n";
        }
      }
      if (n["Building"]["floor"]) {
        std::string s = n["Building"]["floor"].as<std::string>();
        if ((s != "true") && (s != "false")) {
          wentgood = false;
          std::cerr << "\tOption 'Building.floor' invalid; must be 'true' or 'false'.\n";
        }
      }
      if (n["Building"]["inner_walls"]) {
        std::string s = n["Building"]["inner_walls"].as<std::string>();
        if ((s != "true") && (s != "false")) {
          wentgood = false;
          std::cerr << "\tOption 'Building.inner_walls' invalid; must be 'true' or 'false'.\n";
        }
      }
    }
    if (n["Terrain"]) {
      if (n["Terrain"]["simplification"]) {
        if (is_string_integer(n["Terrain"]["simplification"].as<std::string>()) == false) {
          wentgood = false;
          std::cerr << "\tOption 'Terrain.simplification' invalid; must be an integer.\n";
        }
      }
      if (n["Terrain"]["simplification_tinsimp"]) {
        try {
          boost::lexical_cast<float>(n["Terrain"]["simplification_tinsimp"].as<std::string>());
        }
        catch (boost::bad_lexical_cast& e) {
          wentgood = false;
          std::cerr << "\tOption 'Terrain.simplification_tinsimp' invalid; must be a double.\n";
        }
      }
      if (n["Terrain"]["innerbuffer"]) {
        try {
          boost::lexical_cast<float>(n["Terrain"]["innerbuffer"].as<std::string>());
        }
        catch (boost::bad_lexical_cast& e) {
          wentgood = false;
          std::cerr << "\tOption 'Terrain.innerbuffer' invalid; must be a float.\n";
        }
      }        
      if (n["Terrain"]["use_LAS_classes"]) {
        YAML::Node tmp = n["Terrain"]["use_LAS_classes"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
          if (is_string_integer(it2->as<std::string>()) == false) {
            wentgood = false;
            std::cerr << "\tOption 'Terrain.use_LAS_classes' invalid; must be an integer.\n";
          }
        }
      }        
      if (n["Terrain"]["use_LAS_classes_within"]) {
        YAML::Node tmp = n["Terrain"]["use_LAS_classes_within"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
          if (is_string_integer(it2->as<std::string>()) == false) {
            wentgood = false;
            std::cerr << "\tOption 'Terrain.use_LAS_classes_within' invalid; must be an integer.\n";
          }
        }
      }
    }
    if (n["Forest"]) {
      if (n["Forest"]["simplification"]) {
        if (is_string_integer(n["Forest"]["simplification"].as<std::string>()) == false) {
          wentgood = false;
          std::cerr << "\tOption 'Forest.simplification' invalid; must be an integer.\n";
        }
      }
      if (n["Forest"]["simplification_tinsimp"]) {
        try {
          boost::lexical_cast<float>(n["Forest"]["simplification_tinsimp"].as<std::string>());
        }
        catch (boost::bad_lexical_cast& e) {
          wentgood = false;
          std::cerr << "\tOption 'Forest.simplification_tinsimp' invalid; must be a double.\n";
        }
      }
      if (n["Forest"]["innerbuffer"]) {
        try {
          boost::lexical_cast<float>(n["Forest"]["innerbuffer"].as<std::string>());
        }
        catch (boost::bad_lexical_cast& e) {
          wentgood = false;
          std::cerr << "\tOption 'Forest.innerbuffer' invalid; must be a float.\n";
        }
      }      
      if (n["Forest"]["use_LAS_classes"]) {
        YAML::Node tmp = n["Forest"]["use_LAS_classes"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
          if (is_string_integer(it2->as<std::string>()) == false) {
            wentgood = false;
            std::cerr << "\tOption 'Forest.use_LAS_classes' invalid; must be an integer.\n";
          }
        }
      } 
      if (n["Forest"]["use_LAS_classes_within"]) {
        YAML::Node tmp = n["Forest"]["use_LAS_classes_within"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
          if (is_string_integer(it2->as<std::string>()) == false) {
            wentgood = false;
            std::cerr << "\tOption 'Forest.use_LAS_classes_within' invalid; must be an integer.\n";
          }
        }
      }
    }
    if (n["Water"]) {
      if (n["Water"]["height"]) {
        std::string s = n["Water"]["height"].as<std::string>();
        if ((s.substr(0, s.find_first_of("-")) != "percentile") ||
          (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false)) {
          wentgood = false;
          std::cerr << "\tOption 'Water.height' invalid; must be 'percentile-XX'.\n";
        }
      }      
      if (n["Water"]["use_LAS_classes"]) {
        YAML::Node tmp = n["Water"]["use_LAS_classes"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
          if (is_string_integer(it2->as<std::string>()) == false) {
            wentgood = false;
            std::cerr << "\tOption 'Water.use_LAS_classes' invalid; must be an integer.\n";
          }
        }
      }   
      if (n["Water"]["use_LAS_classes_within"]) {
        YAML::Node tmp = n["Water"]["use_LAS_classes_within"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
          if (is_string_integer(it2->as<std::string>()) == false) {
            wentgood = false;
            std::cerr << "\tOption 'Water.use_LAS_classes_within' invalid; must be an integer.\n";
          }
        }
      }
    }
    if (n["Road"]) {
      if (n["Road"]["height"]) {
        std::string s = n["Road"]["height"].as<std::string>();
        if ((s.substr(0, s.find_first_of("-")) != "percentile") ||
          (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false)) {
          wentgood = false;
          std::cerr << "\tOption 'Road.height' invalid; must be 'percentile-XX'.\n";
        }
      }
      if (n["Road"]["filter_outliers"]) {
        std::string s = n["Road"]["filter_outliers"].as<std::string>();
        if ((s != "true") && (s != "false")) {
          wentgood = false;
          std::cerr << "\tOption 'Road.filter_outliers' invalid; must be 'true' or 'false'.\n";
        }
      }
      if (n["Road"]["flatten"]) {
        std::string s = n["Road"]["flatten"].as<std::string>();
        if ((s != "true") && (s != "false")) {
          wentgood = false;
          std::cerr << "\tOption 'Road.flatten' invalid; must be 'true' or 'false'.\n";
        }
      }
      if (n["Road"]["use_LAS_classes"]) {
        YAML::Node tmp = n["Road"]["use_LAS_classes"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
          if (is_string_integer(it2->as<std::string>()) == false) {
            wentgood = false;
            std::cerr << "\tOption 'Road.use_LAS_classes' invalid; must be an integer.\n";
          }
        }
      }
      if (n["Road"]["use_LAS_classes_within"]) {
        YAML::Node tmp = n["Road"]["use_LAS_classes_within"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
          if (is_string_integer(it2->as<std::string>()) == false) {
            wentgood = false;
            std::cerr << "\tOption 'Road.use_LAS_classes_within' invalid; must be an integer.\n";
          }
        }
      }
    }
    if (n["Separation"]) {
      if (n["Separation"]["height"]) {
        std::string s = n["Separation"]["height"].as<std::string>();
        if ((s.substr(0, s.find_first_of("-")) != "percentile") ||
          (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false)) {
          wentgood = false;
          std::cerr << "\tOption 'Separation.height' invalid; must be 'percentile-XX'.\n";
        }
      }
      if (n["Separation"]["use_LAS_classes"]) {
        YAML::Node tmp = n["Separation"]["use_LAS_classes"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
          if (is_string_integer(it2->as<std::string>()) == false) {
            wentgood = false;
            std::cerr << "\tOption 'Separation.use_LAS_classes' invalid; must be an integer.\n";
          }
        }
      }
      if (n["Separation"]["use_LAS_classes_within"]) {
        YAML::Node tmp = n["Separation"]["use_LAS_classes_within"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
          if (is_string_integer(it2->as<std::string>()) == false) {
            wentgood = false;
            std::cerr << "\tOption 'Separation.use_LAS_classes_within' invalid; must be an integer.\n";
          }
        }
      }
    }
    if (n["Bridge/Overpass"]) {
      if (n["Bridge/Overpass"]["height"]) {
        std::string s = n["Bridge/Overpass"]["height"].as<std::string>();
        if ((s.substr(0, s.find_first_of("-")) != "percentile") ||
          (is_string_integer(s.substr(s.find_first_of("-") + 1), 0, 100) == false)) {
          wentgood = false;
          std::cerr << "\tOption 'Bridge/Overpass.height' invalid; must be 'percentile-XX'.\n";
        }
      }
      if (n["Bridge/Overpass"]["use_LAS_classes"]) {
        YAML::Node tmp = n["Bridge/Overpass"]["use_LAS_classes"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
          if (is_string_integer(it2->as<std::string>()) == false) {
            wentgood = false;
            std::cerr << "\tOption 'Bridge/Overpass.use_LAS_classes' invalid; must be an integer.\n";
          }
        }
      }
      if (n["Bridge/Overpass"]["use_LAS_classes_within"]) {
        YAML::Node tmp = n["Bridge/Overpass"]["use_LAS_classes_within"];
        for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
          if (is_string_integer(it2->as<std::string>()) == false) {
            wentgood = false;
            std::cerr << "\tOption 'Bridge/Overpass.use_LAS_classes_within' invalid; must be an integer.\n";
          }
        }
      }
      if (n["Bridge/Overpass"]["flatten"]) {
        std::string s = n["Bridge/Overpass"]["flatten"].as<std::string>();
        if ((s != "true") && (s != "false")) {
          wentgood = false;
          std::cerr << "\tOption 'Bridge/Overpass.flatten' invalid; must be 'true' or 'false'.\n";
        }
      }
    }
  }

  //-- 3. input_elevation
  if (nodes["input_elevation"]) {
    YAML::Node n = nodes["input_elevation"];
    for (auto it = n.begin(); it != n.end(); ++it) {
      YAML::Node tmp = (*it)["omit_LAS_classes"];
      for (auto it2 = tmp.begin(); it2 != tmp.end(); ++it2) {
        if (is_string_integer(it2->as<std::string>()) == false) {
          wentgood = false;
          std::cerr << "\tOption 'input_elevation.omit_LAS_class' invalid; must be an integer.\n";
        }
      }
      if ((*it)["thinning"]) {
        if (is_string_integer((*it)["thinning"].as<std::string>()) == false) {
          wentgood = false;
          std::cerr << "\tOption 'input_elevation.thinning' invalid; must be an integer.\n";
        }
      }
    }
  }
  else {
    std::cerr << "Group 'input_elevation' not defined. \n";
    wentgood = false;
  }

  //-- 4. options  
  if (nodes["options"]) {
    YAML::Node n = nodes["options"];
    if (n["radius_vertex_elevation"]) {
      try {
        boost::lexical_cast<float>(n["radius_vertex_elevation"].as<std::string>());
      }
      catch (boost::bad_lexical_cast& e) {
        wentgood = false;
        std::cerr << "\tOption 'options.radius_vertex_elevation' invalid.\n";
      }
    }   
    if (n["building_radius_vertex_elevation"]) {
      try {
        boost::lexical_cast<float>(n["building_radius_vertex_elevation"].as<std::string>());
      }
      catch (boost::bad_lexical_cast& e) {
        wentgood = false;
        std::cerr << "\tOption 'options.building_radius_vertex_elevation' invalid.\n";
      }
    }
    if (n["threshold_jump_edges"]) {
      try {
        boost::lexical_cast<float>(n["threshold_jump_edges"].as<std::string>());
      }
      catch (boost::bad_lexical_cast& e) {
        wentgood = false;
        std::cerr << "\tOption 'options.threshold_jump_edges' invalid.\n";
      }
    }
    if (n["stitching"]) {
      std::string s = n["stitching"].as<std::string>();
      if ((s != "true") && (s != "false")) {
        wentgood = false;
        std::cerr << "\tOption 'options.stitching' invalid; must be 'true' or 'false'.\n";
      }
    }
    if (n["extent"]) {
      std::vector<std::string> extent_split = stringsplit(n["extent"].as<std::string>(), ',');
      double xmin, xmax, ymin, ymax;
      bool correct = true;
      try {
        xmin = boost::lexical_cast<double>(extent_split[0]);
        ymin = boost::lexical_cast<double>(extent_split[1]);
        xmax = boost::lexical_cast<double>(extent_split[2]);
        ymax = boost::lexical_cast<double>(extent_split[3]);
      }
      catch (boost::bad_lexical_cast& e) {
        correct = false;
        wentgood = false;
      }
      if (!correct || xmin > xmax || ymin > ymax || boost::geometry::area(Box2(Point2(xmin, ymin), Point2(xmax, ymax))) <= 0.0) {
        std::cerr << "ERROR: The supplied extent is not valid: (" << n["extent"].as<std::string>() << ")\n";
      }
    }
  }

  //-- 5. output
  if (nodes["output"]) {
    YAML::Node n = nodes["output"];
    if (n["gdal_driver"] && n["gdal_driver"].as<std::string>().empty()) {
      wentgood = false;
      std::cerr << "\tOutput format GDAL needs gdal_driver setting\n";
    }
  }

  return wentgood;
}
