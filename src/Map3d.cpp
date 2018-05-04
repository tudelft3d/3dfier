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

#include "Map3d.h"
#include "io.h"
#include "boost/locale.hpp"
#include "boost/tokenizer.hpp"
#include "boost/algorithm/string.hpp"
#include "nlohmann-json/json.hpp"

Map3d::Map3d() {
  OGRRegisterAll();
  _building_heightref_roof = 0.9;
  _building_heightref_ground = 0.1;
  _building_triangulate = true;
  _building_lod = 1;
  _building_include_floor = false;
  _use_vertical_walls = false;
  _terrain_simplification = 0;
  _forest_simplification = 0;
  _terrain_simplification_tinsimp = 0.0;
  _forest_simplification_tinsimp = 0.0;
  _terrain_innerbuffer = 0.0;
  _forest_innerbuffer = 0.0;
  _water_heightref = 0.1;
  _road_heightref = 0.5;
  _road_threshold_outliers = 30;
  _separation_heightref = 0.8;
  _bridge_heightref = 0.5;
  _radius_vertex_elevation = 1.0;
  _building_radius_vertex_elevation = 3.0;
  _threshold_jump_edges = 50;
  bg::set<bg::min_corner, 0>(_bbox, 999999);
  bg::set<bg::min_corner, 1>(_bbox, 999999);
  bg::set<bg::max_corner, 0>(_bbox, -999999);
  bg::set<bg::max_corner, 1>(_bbox, -999999);
}

Map3d::~Map3d() {
  _lsFeatures.clear();
}

void Map3d::set_building_heightref_roof(float h) {
  _building_heightref_roof = h;
}

void Map3d::set_building_heightref_ground(float h) {
  _building_heightref_ground = h;
}

void Map3d::set_radius_vertex_elevation(float radius) {
  _radius_vertex_elevation = radius;
}

void Map3d::set_building_radius_vertex_elevation(float radius) {
  _building_radius_vertex_elevation = radius;
}

void Map3d::set_threshold_jump_edges(float threshold) {
  _threshold_jump_edges = int(threshold * 100);
}

void Map3d::set_use_vertical_walls(bool useverticalwalls) {
  _use_vertical_walls = useverticalwalls;
}

void Map3d::set_building_include_floor(bool include) {
  _building_include_floor = include;
}

void Map3d::set_building_triangulate(bool triangulate) {
  _building_triangulate = triangulate;
}

void Map3d::set_building_lod(int lod) {
  _building_lod = lod;
}

void Map3d::set_terrain_simplification(int simplification) {
  _terrain_simplification = simplification;
}

void Map3d::set_forest_simplification(int simplification) {
  _forest_simplification = simplification;
}

void Map3d::set_terrain_simplification_tinsimp(double tinsimp_threshold){
  _terrain_simplification_tinsimp = tinsimp_threshold;
}

void Map3d::set_forest_simplification_tinsimp(double tinsimp_threshold){
  _forest_simplification_tinsimp = tinsimp_threshold;
}

void Map3d::set_terrain_innerbuffer(float innerbuffer) {
  _terrain_innerbuffer = innerbuffer;
}

void Map3d::set_forest_innerbuffer(float innerbuffer) {
  _forest_innerbuffer = innerbuffer;
}

void Map3d::set_water_heightref(float h) {
  _water_heightref = h;
}

void Map3d::set_road_heightref(float h) {
  _road_heightref = h;
}

void Map3d::set_road_threshold_outliers(int t) {
  _road_threshold_outliers = t;
}

void Map3d::set_separation_heightref(float h) {
  _separation_heightref = h;
}

void Map3d::set_bridge_heightref(float h) {
  _bridge_heightref = h;
}

void Map3d::set_requested_extent(double xmin, double ymin, double xmax, double ymax) {
  _requestedExtent = Box2(Point2(xmin, ymin), Point2(xmax, ymax));
}

Box2 Map3d::get_bbox() {
  return _bbox;
}

liblas::Bounds<double> Map3d::get_bounds() {
  double radius = std::max(_radius_vertex_elevation, _building_radius_vertex_elevation);
  liblas::Bounds<double> bounds(_bbox.min_corner().x() - radius, _bbox.min_corner().y() - radius, _bbox.max_corner().x() + radius, _bbox.max_corner().y() + radius);
  return bounds;
}

bool Map3d::get_cityjson(std::string filename) {
  std::cout << "CityJSON" << std::endl;
  nlohmann::json j;
  j["type"] = "CityJSON";
  j["version"] = "0.6";
  j["metadata"] = {};
  j["metadata"]["datasetTitle"] = "my 3dfied map";
  j["metadata"]["pointOfContact"] = "https://3d.bk.tudelft.nl";
  double b[] = {bg::get<bg::min_corner, 0>(_bbox),
                bg::get<bg::min_corner, 1>(_bbox), 
                0,
                bg::get<bg::max_corner, 0>(_bbox),
                bg::get<bg::max_corner, 1>(_bbox), 
                0};
  j["metadata"]["bbox"] = b;
  std::unordered_map< std::string, unsigned long > dPts;
  for (auto& f : _lsFeatures) {
    f->get_cityjson(j, dPts);
  }
  //-- vertices
  std::vector<std::string> thepts;
  thepts.resize(dPts.size());
  for (auto& p : dPts)
    thepts[p.second] = p.first;
  dPts.clear();
  for (auto& p : thepts) {
    std::vector<std::string> c;
    boost::split(c, p, boost::is_any_of(" "));
    j["vertices"].push_back({std::stod(c[0], NULL), std::stod(c[1], NULL), std::stod(c[2], NULL) });
  }
  std::ofstream o(filename);
  // o << j.dump(2) << std::endl;      
  o << j.dump() << std::endl;      
  return true;
}

void Map3d::get_citygml(std::ostream& of) {
  create_citygml_header(of);
  for (auto& f : _lsFeatures) {
    f->get_citygml(of);
    of << "\n";
  }
  of << "</CityModel>\n";
}

void Map3d::get_citygml_multifile(std::string ofname) {
  std::unordered_map<std::string, std::ofstream> ofs;

  for (auto& f : _lsFeatures) {
    std::string filename = ofname + f->get_layername() + ".gml";
    if (ofs.find(filename) == ofs.end()) {
      ofs.emplace(filename, std::ofstream(filename));
      create_citygml_header(ofs[filename]);
    }
    f->get_citygml(ofs[filename]);
    ofs[filename] << "\n";
  }
  for (auto i = ofs.begin(); i != ofs.end(); i++) {
    std::ofstream& of = ofs[(*i).first];
    of << "</CityModel>\n";
    of.close();
  }
}

void Map3d::get_citygml_imgeo(std::ostream& of) {
  create_citygml_imgeo_header(of);
  for (auto& f : _lsFeatures) {
    f->get_citygml_imgeo(of);
    of << "\n";
  }
  of << "</CityModel>\n";
}

void Map3d::get_citygml_imgeo_multifile(std::string ofname) {
  std::unordered_map<std::string, std::ofstream> ofs;

  for (auto& f : _lsFeatures) {
    std::string filename = ofname + f->get_layername() + ".gml";
    if (ofs.find(filename) == ofs.end()) {
      ofs.emplace(filename, std::ofstream(filename));
      create_citygml_imgeo_header(ofs[filename]);
    }
    f->get_citygml_imgeo(ofs[filename]);
    ofs[filename] << "\n";
  }
  for (auto it = ofs.begin(); it != ofs.end(); it++) {
    std::ofstream& of = ofs[(*it).first];
    of << "</CityModel>\n";
    of.close();
  }
}

void Map3d::create_citygml_header(std::ostream& of) {
    of << std::setprecision(3) << std::fixed;
    get_xml_header(of);
    get_citygml_namespaces(of);
    of << "<!-- Automatically generated by 3dfier (https://github.com/tudelft3d/3dfier) -->\n";
    of << "<gml:name>my 3dfied map</gml:name>\n";
    of << "<gml:boundedBy>";
    of << "<gml:Envelope srsDimension=\"3\" srsName=\"urn:ogc:def:crs:EPSG::7415\">";
    of << "<gml:lowerCorner>";
    of << bg::get<bg::min_corner, 0>(_bbox) << " " << bg::get<bg::min_corner, 1>(_bbox) << " 0";
    of << "</gml:lowerCorner>";
    of << "<gml:upperCorner>";
    of << bg::get<bg::max_corner, 0>(_bbox) << " " << bg::get<bg::max_corner, 1>(_bbox) << " 100";
    of << "</gml:upperCorner>";
    of << "</gml:Envelope>";
    of << "</gml:boundedBy>\n";
}

void Map3d::create_citygml_imgeo_header(std::ostream& of) {
    of << std::setprecision(3) << std::fixed;
    get_xml_header(of);
    get_citygml_imgeo_namespaces(of);
    of << "<!-- Automatically generated by 3dfier (https://github.com/tudelft3d/3dfier), a software made with <3 by the 3D geoinformation group, TU Delft -->\n";
    of << "<gml:name>my 3dfied map</gml:name>\n";
    of << "<gml:boundedBy>";
    of << "<gml:Envelope srsDimension=\"3\" srsName=\"urn:ogc:def:crs:EPSG::7415\">";
    of << "<gml:lowerCorner>";
    of << bg::get<bg::min_corner, 0>(_bbox) << " " << bg::get<bg::min_corner, 1>(_bbox) << " 0";
    of << "</gml:lowerCorner>";
    of << "<gml:upperCorner>";
    of << bg::get<bg::max_corner, 0>(_bbox) << " " << bg::get<bg::max_corner, 1>(_bbox) << " 100";
    of << "</gml:upperCorner>";
    of << "</gml:Envelope>";
    of << "</gml:boundedBy>\n";
}

void Map3d::get_csv_buildings(std::ostream& of) {
  of << "id,roof,ground\n";
  for (auto& p : _lsFeatures) {
    if (p->get_class() == BUILDING) {
      Building* b = dynamic_cast<Building*>(p);
      b->get_csv(of);
    }
  }
}

void Map3d::get_csv_buildings_all_elevation_points(std::ostream &outputfile) {
  outputfile << "id,allzvalues" << std::endl;
  for (auto& p : _lsFeatures) {
    if (p->get_class() == BUILDING) {
      Building* b = dynamic_cast<Building*>(p);
      outputfile << b->get_id() << ",";
      outputfile << b->get_all_z_values();
      outputfile << "," << std::endl;
    }
  }
}

void Map3d::get_csv_buildings_multiple_heights(std::ostream &outputfile) {
  //-- ground heights
  std::vector<float> gpercentiles = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f};
  std::vector<float> rpercentiles = {0.0f, 0.1f, 0.25f, 0.5f, 0.75f, 0.9f, 0.95f, 0.99f};
  outputfile << std::setprecision(2) << std::fixed;
  outputfile << "id,";
  for (auto& each : gpercentiles)
    outputfile << "ground-" << each << ",";
  for (auto& each : rpercentiles)
    outputfile << "roof-" << each << ",";
  outputfile << std::endl;
  for (auto& p : _lsFeatures) {
    if (p->get_class() == BUILDING) {
      Building* b = dynamic_cast<Building*>(p);
      outputfile << b->get_id() << ",";
      for (auto& each : gpercentiles) {
        int h = b->get_height_ground_at_percentile(each);
        outputfile << float(h)/100 << ",";
      }
      for (auto& each : rpercentiles) {
        int h = b->get_height_roof_at_percentile(each);
        outputfile << float(h)/100 << ",";
      }
      outputfile << std::endl;
    }
  }
}

void Map3d::get_obj_per_feature(std::ostream& of, int z_exaggeration) {
  std::unordered_map< std::string, unsigned long > dPts;
  std::string fs;
  
  for (auto& p : _lsFeatures) {
    fs += "o "; fs += p->get_id(); fs += "\n";
    if (p->get_class() == BUILDING) {
      Building* b = dynamic_cast<Building*>(p);
      b->get_obj(dPts, _building_lod, b->get_mtl(), fs);
    }
    else {
      p->get_obj(dPts, p->get_mtl(), fs);
    }
  }

  //-- sort the points in the map: simpler to copy to a vector
  std::vector<std::string> thepts;
  thepts.resize(dPts.size());
  for (auto& p : dPts)
    thepts[p.second - 1] = p.first;
  dPts.clear();

  of << "mtllib ./3dfier.mtl" << "\n";
  for (auto& p : thepts) {
    of << "v " << p << "\n";
  }
  of << fs << std::endl;
}

void Map3d::get_obj_per_class(std::ostream& of, int z_exaggeration) {
  std::unordered_map< std::string, unsigned long > dPts;
  std::string fs;
  for (int c = 0; c < 6; c++) {
    for (auto& p : _lsFeatures) {
      if (p->get_class() == c) {
        if (p->get_class() == BUILDING) {
          Building* b = dynamic_cast<Building*>(p);
          b->get_obj(dPts, _building_lod, b->get_mtl(), fs);
        }
        else {
          p->get_obj(dPts, p->get_mtl(), fs);
        }
      }
    }
  }

  //-- sort the points in the map: simpler to copy to a vector
  std::vector<std::string> thepts;
  thepts.resize(dPts.size());
  for (auto& p : dPts)
    thepts[p.second - 1] = p.first;
  dPts.clear();

  of << "mtllib ./3dfier.mtl\n";
  for (auto& p : thepts) {
    of << "v " << p << std::endl;
  }
  of << fs << std::endl;
}

bool Map3d::get_pdok_output(std::string filename) {
#if GDAL_VERSION_MAJOR < 2
  std::cerr << "ERROR: cannot write MultiPolygonZ files with GDAL < 2.0.\n";
  return false;
#else
  if (GDALGetDriverCount() == 0)
    GDALAllRegister();
  GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("PostgreSQL");

  std::unordered_map<std::string, OGRLayer*> layers;

  for (auto& f : _lsFeatures) {
    std::string layername = f->get_layername();
    if (layers.find(layername) == layers.end()) {
      std::string tmpFilename = filename;
      AttributeMap attributes = f->get_attributes();
      //Add additional attribute to list for layer creation
      attributes["xml"] = std::make_pair(OFTString, "");
      OGRLayer *layer = create_gdal_layer(driver, tmpFilename, layername, attributes, f->get_class() == BUILDING);
      if (layer == NULL) {
        std::cerr << "ERROR: Cannot open database '" + filename + "' for writing" << std::endl;
        for (auto& layer : layers) {
          GDALClose(layer.second);
        }
        GDALClose(driver);
        return false;
      }
      layers.emplace(layername, layer);
    }

    //Add additional attribute describing CityGML of feature
    std::stringstream ss;
    f->get_citygml_imgeo(ss);
    std::string gmlAttribute = ss.str();
    ss.clear();
    AttributeMap extraAttribute = AttributeMap();
    extraAttribute["xml"] = std::make_pair(OFTString, gmlAttribute);

    f->get_shape(layers[layername], true, extraAttribute);
  }
  for (auto& layer : layers) {
    GDALClose(layer.second);
  }
  GDALClose(driver);
  return true;
#endif
}

bool Map3d::get_gdal_output(std::string filename, std::string drivername, bool multi) {
#if GDAL_VERSION_MAJOR < 2
  std::cerr << "ERROR: cannot write MultiPolygonZ files with GDAL < 2.0.\n";
  return false;
#else
  if (GDALGetDriverCount() == 0)
    GDALAllRegister();
  GDALDriver *driver = GetGDALDriverManager()->GetDriverByName(drivername.c_str());

  if (!multi) {
    OGRLayer *layer = create_gdal_layer(driver, filename, "my3dmap", AttributeMap(), true);
    if (layer == NULL) {
      std::cerr << "ERROR: Cannot open file '" + filename + "' for writing" << std::endl;
      GDALClose(layer);
      GDALClose(driver);
      return false;
    }
    for (auto& f : _lsFeatures) {
      f->get_shape(layer, false);
    }
    GDALClose(layer);
    GDALClose(driver);
  }
  else {
    std::unordered_map<std::string, OGRLayer*> layers;

    for (auto& f : _lsFeatures) {
      std::string layername = f->get_layername();
      if (layers.find(layername) == layers.end()) {
        std::string tmpFilename = filename;
        if (drivername == "ESRI Shapefile") {
          tmpFilename = filename + layername;
        }
        OGRLayer *layer = create_gdal_layer(driver, tmpFilename, layername, f->get_attributes(), f->get_class() == BUILDING);
        if (layer == NULL) {
          std::cerr << "ERROR: Cannot open file '" + filename + "' for writing" << std::endl;
          for (auto& layer : layers) {
            GDALClose(layer.second);
          }
          GDALClose(driver);
          return false;
        }
        layers.emplace(layername, layer);
      }
      f->get_shape(layers[layername], true);
    }
    for (auto& layer : layers) {
      GDALClose(layer.second);
    }
    GDALClose(driver);
  }
  return true;
#endif
}

#if GDAL_VERSION_MAJOR >= 2
OGRLayer* Map3d::create_gdal_layer(GDALDriver *driver, std::string filename, std::string layername, AttributeMap attributes, bool addHeightAttributes) {
  GDALDataset *dataSource = driver->Create(filename.c_str(), 0, 0, 0, GDT_Unknown, NULL);

  if (dataSource == NULL) {
    std::cerr << "ERROR: could not open file, skipping it.\n";
    return NULL;
  }
  OGRLayer *layer = dataSource->GetLayerByName(layername.c_str());
  if (layer == NULL) {
    OGRSpatialReference* sr = new OGRSpatialReference();
    sr->importFromEPSG(7415);
    layer = dataSource->CreateLayer(layername.c_str(), sr, OGR_GT_SetZ(wkbMultiPolygon), NULL);

    OGRFieldDefn oField("3dfier_Id", OFTString);
    if (layer->CreateField(&oField) != OGRERR_NONE) {
      std::cerr << "Creating 3dfier_Id field failed.\n";
      return NULL;
    }
    OGRFieldDefn oField2("3dfier_Class", OFTString);
    if (layer->CreateField(&oField2) != OGRERR_NONE) {
      std::cerr << "Creating 3dfier_Class field failed.\n";
      return NULL;
    }
    if (addHeightAttributes) {
      OGRFieldDefn oField3("BaseHeight", OFTReal);
      if (layer->CreateField(&oField3) != OGRERR_NONE) {
        std::cerr << "Creating BaseHeight field failed.\n";
        return NULL;
      }
      OGRFieldDefn oField4("RoofHeight", OFTReal);
      if (layer->CreateField(&oField4) != OGRERR_NONE) {
        std::cerr << "Creating RoofHeight field failed.\n";
        return NULL;
      }
    }
    for (auto attr : attributes) {
      OGRFieldDefn oField(attr.first.c_str(), attr.second.first);
      if (layer->CreateField(&oField) != OGRERR_NONE) {
        std::cerr << "Creating " + attr.first + " field failed.\n";
        return NULL;
      }
    }
  }
  return layer;
}
#endif

bool Map3d::get_shapefile2d(std::string filename) {
#if GDAL_VERSION_MAJOR < 2
  if (OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount() == 0)
    OGRRegisterAll();
  OGRSFDriver *driver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
  OGRDataSource *dataSource = driver->CreateDataSource(filename.c_str(), NULL);
#else
  if (GDALGetDriverCount() == 0)
    GDALAllRegister();
  GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
  GDALDataset *dataSource = driver->Create(filename.c_str(), 0, 0, 0, GDT_Unknown, NULL);
#endif

  if (dataSource == NULL) {
    std::cerr << "\tERROR: could not open file, skipping it.\n";
    return false;
  }
  OGRLayer *layer = dataSource->CreateLayer("my3dmap", NULL, wkbMultiPolygon, NULL);

  OGRFieldDefn oField("3dfier_Id", OFTString);
  if (layer->CreateField(&oField) != OGRERR_NONE) {
    std::cerr << "Creating 3dfier_Id field failed.\n";
    return false;
  }
  OGRFieldDefn oField2("3dfier_Class", OFTString);
  if (layer->CreateField(&oField2) != OGRERR_NONE) {
    std::cerr << "Creating 3dfier_Class field failed.\n";
    return false;
  }
  OGRFieldDefn oField3("BaseHeight", OFTReal);
  if (layer->CreateField(&oField3) != OGRERR_NONE) {
    std::cerr << "Creating FloorHeight field failed.\n";
    return false;
  }
  OGRFieldDefn oField4("RoofHeight", OFTReal);
  if (layer->CreateField(&oField4) != OGRERR_NONE) {
    std::cerr << "Creating RoofHeight field failed.\n";
    return false;
  }
  for (auto& f : _lsFeatures) {
    f->get_shape(layer, false);
  }
#if GDAL_VERSION_MAJOR < 2
  OGRDataSource::DestroyDataSource(dataSource);
#else
  GDALClose(dataSource);
#endif
  return true;
}

unsigned long Map3d::get_num_polygons() {
  return _lsFeatures.size();
}

const std::vector<TopoFeature*>& Map3d::get_polygons3d() {
  return _lsFeatures;
}

void Map3d::add_elevation_point(liblas::Point const& laspt) {
  std::vector<PairIndexed> re;
  Point2 minp(laspt.GetX() - _radius_vertex_elevation, laspt.GetY() - _radius_vertex_elevation);
  Point2 maxp(laspt.GetX() + _radius_vertex_elevation, laspt.GetY() + _radius_vertex_elevation);
  Box2 querybox(minp, maxp);
  _rtree.query(bgi::intersects(querybox), std::back_inserter(re));
  minp = Point2(laspt.GetX() - _building_radius_vertex_elevation, laspt.GetY() - _building_radius_vertex_elevation);
  maxp = Point2(laspt.GetX() + _building_radius_vertex_elevation, laspt.GetY() + _building_radius_vertex_elevation);
  querybox = Box2(minp, maxp);
  _rtree_buildings.query(bgi::intersects(querybox), std::back_inserter(re));

  for (auto& v : re) {
    TopoFeature* f = v.second;
    float radius = _radius_vertex_elevation;
    //-- only process last returns; 
    //-- although perhaps not smart for vegetation/forest in the future
    //-- TODO: always ignore the non-last-return points?
    if (laspt.GetReturnNumber() != laspt.GetNumberOfReturns()) 
      continue;

    int c = laspt.GetClassification().GetClass();
    bool bInsert = false;
    if (f->get_class() == BUILDING) {
      bInsert = true;
      radius = _building_radius_vertex_elevation;
    }
    else if (f->get_class() == TERRAIN) {
      if (_las_classes_allowed[LAS_TERRAIN].empty() || _las_classes_allowed[LAS_TERRAIN].count(c) > 0) {
        bInsert = true;
      }
    }
    else if (f->get_class() == FOREST) {
      if (_las_classes_allowed[LAS_FOREST].empty() || _las_classes_allowed[LAS_FOREST].count(c) > 0) {
        bInsert = true;
      }
    }
    else if (f->get_class() == ROAD) {
      if (_las_classes_allowed[LAS_ROAD].empty() || _las_classes_allowed[LAS_ROAD].count(c) > 0) {
        bInsert = true;
      }
    }
    else if (f->get_class() == WATER) {
      if (_las_classes_allowed[LAS_WATER].empty() || _las_classes_allowed[LAS_WATER].count(c) > 0) {
        bInsert = true;
      }
    }
    else if (f->get_class() == SEPARATION) {
      if (_las_classes_allowed[LAS_SEPARATION].empty() || _las_classes_allowed[LAS_SEPARATION].count(c) > 0) {
        bInsert = true;
      }
    }
    else if (f->get_class() == BRIDGE) {
      if (_las_classes_allowed[LAS_BRIDGE].empty() || _las_classes_allowed[LAS_BRIDGE].count(c) > 0) {
        bInsert = true;
      }
    }
 
    if (bInsert == true) { //-- only insert if in the allowed LAS classes
      Point2 p(laspt.GetX(), laspt.GetY());
      f->add_elevation_point(p, laspt.GetZ(), radius, c); 
    }
  }
}

bool Map3d::threeDfy(bool stitching) {
  /*
    1. lift
    2. stitch
    3. process vertical walls
  */
  try {
    std::clog << "===== /LIFTING =====\n";
    for (auto& f : _lsFeatures) {
      f->lift();
    }
    std::clog << "===== LIFTING/ =====\n";
    if (stitching == true) {
      std::clog << "=====  /ADJACENT FEATURES =====\n";
      for (auto& f : _lsFeatures) {
        this->collect_adjacent_features(f);
      }
      std::clog << "=====  ADJACENT FEATURES/ =====\n";

      std::clog << "=====  /STITCHING =====\n";
      this->stitch_lifted_features();
      //-- handle bridges seperately
      this->stitch_bridges();
      std::clog << "=====  STITCHING/ =====\n";

      //-- Sort all node column vectors
      for (auto& nc : _nc) {
        std::sort(nc.second.begin(), nc.second.end());
      }

      std::clog << "=====  /BOWTIES =====\n";
      for (auto& f : _lsFeatures) {
        if (f->has_vertical_walls() == true) {
          f->fix_bowtie();
        }
      }
      std::clog << "=====  BOWTIES/ =====\n";

      std::clog << "=====  /VERTICAL WALLS =====\n";
      for (auto& f : _lsFeatures) {
        if (f->has_vertical_walls() == true) {
          int baseheight = 0;
          if (f->get_class() == BUILDING) {
            baseheight = dynamic_cast<Building*>(f)->get_height_base();
          }
          f->construct_vertical_walls(_nc, baseheight);
        }
      }
      std::clog << "=====  VERTICAL WALLS/ =====\n";
    }
  }
  catch (std::exception e) {
    std::cerr << std::endl << "3dfying failed with error: " << e.what() << std::endl;
    return false;
  }
  return true;
}

bool Map3d::construct_CDT() {
  std::clog << "=====  /CDT =====\n";
  for (auto& p : _lsFeatures) {
    try {
      p->buildCDT();
    }
    catch (std::exception e) {
      std::cerr << std::endl << "CDT failed for " << p->get_id() << " (" << p->get_class() << ") with error: " << e.what() << std::endl;
      return false;
    }
  }
  std::clog << "=====  CDT/ =====\n";
  return true;
}

bool Map3d::save_building_variables() {
  Building::set_las_classes_roof(_las_classes_allowed[LAS_BUILDING_ROOF]);
  Building::set_las_classes_ground(_las_classes_allowed[LAS_BUILDING_GROUND]);
  return true;
}

bool Map3d::construct_rtree() {
  std::clog << "Constructing the R-tree...";
  for (auto p : _lsFeatures) {
    if (p->get_class() == BUILDING) {
      _rtree_buildings.insert(std::make_pair(p->get_bbox2d(), p));
    }
    else {
      _rtree.insert(std::make_pair(p->get_bbox2d(), p));
    }
  }
  std::clog << " done.\n";

  //-- update the bounding box from _rtree and _rtree_buildings 
  _bbox = Box2(
    Point2(std::min(bg::get<bg::min_corner, 0>(_rtree.bounds()), bg::get<bg::min_corner, 0>(_rtree_buildings.bounds())),
      std::min(bg::get<bg::min_corner, 1>(_rtree.bounds()), bg::get<bg::min_corner, 1>(_rtree_buildings.bounds()))),
    Point2(std::max(bg::get<bg::max_corner, 0>(_rtree.bounds()), bg::get<bg::max_corner, 0>(_rtree_buildings.bounds())),
      std::max(bg::get<bg::max_corner, 1>(_rtree.bounds()), bg::get<bg::max_corner, 1>(_rtree_buildings.bounds()))));
  return true;
}

bool Map3d::add_polygons_files(std::vector<PolygonFile> &files) {
#if GDAL_VERSION_MAJOR < 2
  if (OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount() == 0)
    OGRRegisterAll();
#else
  if (GDALGetDriverCount() == 0)
    GDALAllRegister();
#endif

  for (auto file = files.begin(); file != files.end(); ++file) {
    std::clog << "Reading input dataset: " << file->filename << std::endl;
#if GDAL_VERSION_MAJOR < 2
    OGRDataSource *dataSource = OGRSFDriverRegistrar::Open(file->filename.c_str(), false);
#else
    GDALDataset *dataSource = (GDALDataset*)GDALOpenEx(file->filename.c_str(), GDAL_OF_READONLY, NULL, NULL, NULL);
#endif

    if (dataSource == NULL) {
      std::cerr << "\tERROR: could not open file: " + file->filename << std::endl;
      return false;
    }

    // if the file doesn't have layers specified, add all
    if (file->layers[0].first.empty()) {
      std::string lifting = file->layers[0].second;
      file->layers.clear();
      int numberOfLayers = dataSource->GetLayerCount();
      for (int i = 0; i < numberOfLayers; i++) {
        OGRLayer *dataLayer = dataSource->GetLayer(i);
        file->layers.emplace_back(dataLayer->GetName(), lifting);
      }
    }
    bool wentgood = this->extract_and_add_polygon(dataSource, &(*file));
#if GDAL_VERSION_MAJOR < 2
    OGRDataSource::DestroyDataSource(dataSource);
#else
    GDALClose(dataSource);
#endif

    if (!wentgood) {
      return false;
    }
  }
  return true;
}

#if GDAL_VERSION_MAJOR < 2
bool Map3d::extract_and_add_polygon(OGRDataSource* dataSource, PolygonFile* file) {
#else
bool Map3d::extract_and_add_polygon(GDALDataset* dataSource, PolygonFile* file) {
#endif
  const char *idfield = file->idfield.c_str();
  const char *heightfield = file->heightfield.c_str();
  bool multiple_heights = file->handle_multiple_heights;
  bool wentgood = false;
  for (auto l : file->layers) {
    OGRLayer *dataLayer = dataSource->GetLayerByName((l.first).c_str());
    if (dataLayer == NULL) {
      continue;
    }
    if (dataLayer->FindFieldIndex(idfield, false) == -1) {
      std::cerr << "ERROR: field '" << idfield << "' not found in layer '" << l.first << "'.\n";
      return false;
    }
    if (dataLayer->FindFieldIndex(heightfield, false) == -1) {
      std::clog << "Warning: field '" << heightfield << "' not found in layer '" << l.first << "', using all polygons.\n";
    }
    dataLayer->ResetReading();
    unsigned int numberOfPolygons = dataLayer->GetFeatureCount(true);
    std::string layerName = dataLayer->GetName();
    std::clog << "\tLayer: " << layerName << std::endl;
    std::clog << "\t(" << boost::locale::as::number << numberOfPolygons << " features --> " << l.second << ")\n";
    OGRFeature *f;

    //-- check if extent is given and polygons need filtering
    bool useRequestedExtent = false;
    OGREnvelope envelope = OGREnvelope();
    if (boost::geometry::area(_requestedExtent) > 0) {
      envelope.MinX = bg::get<bg::min_corner, 0>(_requestedExtent);
      envelope.MaxX = bg::get<bg::max_corner, 1>(_requestedExtent);
      envelope.MinY = bg::get<bg::min_corner, 0>(_requestedExtent);
      envelope.MaxY = bg::get<bg::max_corner, 1>(_requestedExtent);
      useRequestedExtent = true;
    }

    int numSplitMulti = 0;
    int numSplitPoly = 0;
    while ((f = dataLayer->GetNextFeature()) != NULL) {
      OGRGeometry *geometry = f->GetGeometryRef();
      if (!geometry->IsValid()) {
        std::cerr << "Geometry invalid: " << f->GetFieldAsString(idfield) << std::endl;
      }
      OGREnvelope env;
      if (useRequestedExtent) {
        geometry->getEnvelope(&env);
      }

      //-- add the polygon of no extent is used or if the envelope is within the extent
      if (!useRequestedExtent || envelope.Intersects(env)) {
        switch (geometry->getGeometryType()) {
        case wkbPolygon:
        case wkbPolygon25D: {
          extract_feature(f, layerName, idfield, heightfield, l.second, multiple_heights);
          break;
        }
        case wkbMultiPolygon:
        case wkbMultiPolygon25D: {
          OGRMultiPolygon* multipolygon = (OGRMultiPolygon*)geometry;
          int numGeom = multipolygon->getNumGeometries();
          if (numGeom >= 1) {
            for (int i = 0; i < numGeom; i++) {
              OGRFeature* cf = f->Clone();
              if (numGeom > 1) {
                std::string idString = (std::string)f->GetFieldAsString(idfield) + "-" + std::to_string(i);
                cf->SetField(idfield, idString.c_str());
              }
              cf->SetGeometry((OGRPolygon*)multipolygon->getGeometryRef(i));
              extract_feature(cf, layerName, idfield, heightfield, l.second, multiple_heights);
            }
            numSplitMulti++;
            numSplitPoly += numGeom;
          }
          break;
        }
        default: {
          continue;
        }
        }
      }
    }
    if (numSplitMulti > 0) {
      std::clog << "\tSplit " << numSplitMulti << " MultiPolygon(s) into " << numSplitPoly << " Polygon(s)\n";
    }
    wentgood = true;
  }
  return wentgood;
}

void Map3d::extract_feature(OGRFeature *f, std::string layername, const char *idfield, const char *heightfield, std::string layertype, bool multiple_heights) {
  char *wkt;
  OGRGeometry *geom = f->GetGeometryRef();
  geom->flattenTo2D();
  geom->exportToWkt(&wkt);
  AttributeMap attributes;
  int attributeCount = f->GetFieldCount();
  for (int i = 0; i < attributeCount; i++) {
    attributes[boost::locale::to_lower(f->GetFieldDefnRef(i)->GetNameRef())] = std::make_pair(f->GetFieldDefnRef(i)->GetType(), f->GetFieldAsString(i));
  }
  if (layertype == "Building") {
    Building* p3 = new Building(wkt, layername, attributes, f->GetFieldAsString(idfield), _building_heightref_roof, _building_heightref_ground);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Terrain") {
    Terrain* p3 = new Terrain(wkt, layername, attributes, f->GetFieldAsString(idfield), this->_terrain_simplification, this->_terrain_simplification_tinsimp, this->_terrain_innerbuffer);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Forest") {
    Forest* p3 = new Forest(wkt, layername, attributes, f->GetFieldAsString(idfield), this->_forest_simplification, this->_forest_simplification_tinsimp, this->_forest_innerbuffer);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Water") {
    Water* p3 = new Water(wkt, layername, attributes, f->GetFieldAsString(idfield), this->_water_heightref);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Road") {
    Road* p3 = new Road(wkt, layername, attributes, f->GetFieldAsString(idfield), this->_road_heightref, this->_road_threshold_outliers);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Separation") {
    Separation* p3 = new Separation(wkt, layername, attributes, f->GetFieldAsString(idfield), this->_separation_heightref);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Bridge/Overpass") {
    Bridge* p3 = new Bridge(wkt, layername, attributes, f->GetFieldAsString(idfield), this->_bridge_heightref);
    _lsFeatures.push_back(p3);
  }
  //-- flag all polygons at (niveau != 0) or remove if not handling multiple height levels
  if ((f->GetFieldIndex(heightfield) != -1) && (f->GetFieldAsInteger(heightfield) != 0)) {
    if (multiple_heights) {
      // std::clog << "niveau=" << f->GetFieldAsInteger(heightfield) << ": " << f->GetFieldAsString(idfield) << std::endl;
      _lsFeatures.back()->set_top_level(false);
    }
    else {
      _lsFeatures.pop_back();
    }
  }
}

//-- http://www.liblas.org/tutorial/cpp.html#applying-filters-to-a-reader-to-extract-specified-classes
bool Map3d::add_las_file(PointFile pointFile) {
  std::clog << "Reading LAS/LAZ file: " << pointFile.filename << std::endl;
  std::ifstream ifs;
  ifs.open(pointFile.filename.c_str(), std::ios::in | std::ios::binary);
  if (ifs.is_open() == false) {
    std::cerr << "\tERROR: could not open file: " << pointFile.filename << std::endl;
    return false;
  }
  //-- LAS classes to omit
  std::vector<liblas::Classification> liblasomits;
  for (int i : pointFile.lasomits) {
    liblasomits.push_back(liblas::Classification(i));
  }

  //-- read each point 1-by-1
  liblas::ReaderFactory f;
  liblas::Reader reader = f.CreateWithStream(ifs);
  liblas::Header const& header = reader.GetHeader();

  //-- check if the file overlaps the polygons
  liblas::Bounds<double> bounds = header.GetExtent();
  liblas::Bounds<double> polygonBounds = get_bounds();
  uint32_t pointCount = header.GetPointRecordsCount();
  if (polygonBounds.intersects(bounds)) {
    std::clog << "\t(" << boost::locale::as::number << pointCount << " points in the file)\n";
    if ((pointFile.thinning > 1)) {
      std::clog << "\t(skipping every " << pointFile.thinning << "th points, thus ";
      std::clog << boost::locale::as::number << (pointCount / pointFile.thinning) << " are used)\n";
    }
    else
      std::clog << "\t(all points used, no skipping)\n";

    if (pointFile.lasomits.empty() == false) {
      std::clog << "\t(omitting LAS classes: ";
      for (int i : pointFile.lasomits)
        std::clog << i << " ";
      std::clog << ")\n";
    }
    printProgressBar(0);
    int i = 0;
    
    try {
      while (reader.ReadNextPoint()) {
        liblas::Point const& p = reader.GetPoint();
        //-- set the thinning filter
        if (i % pointFile.thinning == 0) {
          //-- set the classification filter
          if (std::find(liblasomits.begin(), liblasomits.end(), p.GetClassification()) == liblasomits.end()) {
            //-- set the bounds filter
            if (polygonBounds.contains(p)) {
              this->add_elevation_point(p);
            }
          }
        }
        if (i % (pointCount / 100) == 0)
          printProgressBar(100 * (i / double(pointCount)));
        i++;
      }
      printProgressBar(100);
      std::clog << std::endl;
    }
    catch (std::exception e) {
      std::cerr << std::endl << e.what() << std::endl;
      ifs.close();
      return false;
    }
  }
  else {
    std::clog << "\tskipping file, bounds do not intersect polygon extent\n";
  }
  ifs.close();
  return true;
}

void Map3d::collect_adjacent_features(TopoFeature* f) {
  std::vector<PairIndexed> re;
  _rtree.query(bgi::intersects(f->get_bbox2d()), std::back_inserter(re));
  _rtree_buildings.query(bgi::intersects(f->get_bbox2d()), std::back_inserter(re));
  for (auto& each : re) {
    TopoFeature* fadj = each.second;
    if (f != fadj && f->adjacent(*(fadj->get_Polygon2()))){
      f->add_adjacent_feature(fadj);
    }
  }
}

void Map3d::stitch_lifted_features() {
  std::vector<int> ringis, pis;
  for (auto& f : _lsFeatures) {
    if (f->get_class() != BRIDGE) {
      //-- 1. store all touching top level (adjacent + incident)
      std::vector<TopoFeature*>* lstouching = f->get_adjacent_features();
      //-- 2. build the node-column for each vertex
      // oring
      Ring2 oring = f->get_Polygon2()->outer();
      for (int i = 0; i < oring.size(); i++) {
        std::vector< std::tuple<TopoFeature*, int, int> > star;
        bool toprocess = false;
        for (auto& fadj : *lstouching) {
          ringis.clear();
          pis.clear();
          if (fadj->has_point2(oring[i], ringis, pis) == true) {
            for (int k = 0; k < ringis.size(); k++) {
              toprocess = true;
              star.push_back(std::make_tuple(fadj, ringis[k], pis[k]));
            }
          }
        }
        if (toprocess == true) {
          this->stitch_one_vertex(f, 0, i, star);
        }
        else if (f->get_class() == BUILDING) {
          f->add_vertical_wall();
          Point2 tmp = f->get_point2(0, i);
          std::string key_bucket = gen_key_bucket(&tmp);
          int z = f->get_vertex_elevation(0, i);
          _nc[key_bucket].push_back(z);
          z = dynamic_cast<Building*>(f)->get_height_base();
          _nc[key_bucket].push_back(z);
        }
      }
      // irings
      int noiring = 0;
      for (Ring2& iring : f->get_Polygon2()->inners()) {
        noiring++;
        for (int i = 0; i < iring.size(); i++) {
          std::vector< std::tuple<TopoFeature*, int, int> > star;
          bool toprocess = false;
          for (auto& fadj : *lstouching) {
            ringis.clear();
            pis.clear();
            if (fadj->has_point2(iring[i], ringis, pis) == true) {
              for (int k = 0; k < ringis.size(); k++) {
                toprocess = true;
                star.push_back(std::make_tuple(fadj, ringis[k], pis[k]));
              }
            }
          }
          if (toprocess == true) {
            this->stitch_one_vertex(f, noiring, i, star);
          }
          else if (f->get_class() == BUILDING) {
            f->add_vertical_wall();
            Point2 tmp = f->get_point2(0, i);
            std::string key_bucket = gen_key_bucket(&tmp);
            int z = f->get_vertex_elevation(0, i);
            _nc[key_bucket].push_back(z);
            z = dynamic_cast<Building*>(f)->get_height_base();
            _nc[key_bucket].push_back(z);
          }
        }
      }
    }
  }
}

void Map3d::stitch_one_vertex(TopoFeature* f, int ringi, int pi, std::vector< std::tuple<TopoFeature*, int, int> >& star) {
  //-- degree of vertex == 2
  if (star.size() == 1){
    if (std::get<0>(star[0])->get_class() != BRIDGE) {
      TopoFeature* fadj = std::get<0>(star[0]);
      //-- if not building and same class or both soft, then average.
      if (f->get_class() != BUILDING && (f->is_hard() == false && fadj->is_hard() == false)) {
        stitch_average(f, ringi, pi, fadj, std::get<1>(star[0]), std::get<2>(star[0]));
      }
      else {
        stitch_jumpedge(f, ringi, pi, fadj, std::get<1>(star[0]), std::get<2>(star[0]));
      }
    }
    else {
      // for bridges we need to create VW and therefor add the height to the NC
      Point2 p = f->get_point2(ringi, pi);
      _nc[gen_key_bucket(&p)].push_back(f->get_vertex_elevation(ringi, pi));
    }
  }
  //-- degree of vertex >= 3: more complex cases
  else if (star.size() > 1) {
    //-- collect all elevations
    std::vector< std::tuple< int, TopoFeature*, int, int > > zstar;
    zstar.push_back(std::make_tuple(
      f->get_vertex_elevation(ringi, pi),
      f,
      ringi,
      pi));
    for (auto& fadj : star) {
      if (std::get<0>(fadj)->get_class() != BRIDGE) {
        zstar.push_back(std::make_tuple(
          std::get<0>(fadj)->get_vertex_elevation(std::get<1>(fadj), std::get<2>(fadj)),
          std::get<0>(fadj),
          std::get<1>(fadj),
          std::get<2>(fadj)));
      }
      // This it for adjacent objects at the corners of the bridge where the adjacent features need extra VW at the height jump.
      else {
        f->add_vertical_wall();
      }
    }

    //-- sort low-high based on heights (get<0>)
    std::sort(zstar.begin(), zstar.end(),
      [](std::tuple<int, TopoFeature*, int, int> const &t1, std::tuple<int, TopoFeature*, int, int> const &t2) {
      return std::get<0>(t1) < std::get<0>(t2);
    });

    //-- Identify buildings and water
    int building = -1;
    int water = -1;
    for (int i = 0; i < zstar.size(); i++) {
      TopoClass topoClass = std::get<1>(zstar[i])->get_class();
      if (topoClass == BUILDING) {
        //-- set building to the one with the lowest base
        if (building == -1 || dynamic_cast<Building*>(std::get<1>(zstar[building]))->get_height_base() > dynamic_cast<Building*>(std::get<1>(zstar[i]))->get_height_base()) {
          building = i;
        }
      }
      else if (topoClass == WATER) {
        water = i;
      }
    }

    //-- Deal with buildings. If there's a building and a soft class incident, then this soft class
    //-- get allocated the height value of the floor of the building. Any building will do if >1.
    //-- Also ignore water so it doesn't get snapped to the floor of a building
    if (building != -1) {
      int baseheight = dynamic_cast<Building*>(std::get<1>(zstar[building]))->get_height_base();
      for (auto& each : zstar) {
        if (std::get<1>(each)->get_class() != BUILDING && std::get<1>(each)->get_class() != WATER) {
          std::get<0>(each) = baseheight;
          if (water != -1) {
            //- add a vertical wall between the feature and the water
            std::get<1>(each)->add_vertical_wall();
          }
        }
        else if (std::get<1>(each)->get_class() == BUILDING) {
          std::get<1>(each)->add_vertical_wall();
        }
      }
    }
    else {
      for (std::vector< std::tuple< int, TopoFeature*, int, int > >::iterator it = zstar.begin(); it != zstar.end(); ++it) {
        std::vector< std::tuple< int, TopoFeature*, int, int > >::iterator fnext = it;
        for (std::vector< std::tuple< int, TopoFeature*, int, int > >::iterator it2 = it + 1; it2 != zstar.end(); ++it2) {
          int deltaz = std::abs(std::get<0>(*it) - std::get<0>(*it2));
          // features are within threshold jump edge, handle various cases
          if (deltaz < this->_threshold_jump_edges) {
            fnext = it2;
            // it and it2 are same class, set height to first since averaging doesn't work if >2 objects of same class within threshold
            // this mainly applies for bridges and outlier detection of roads, otherwise it shouldn't be possible
            if (std::get<1>(*it)->get_class() == std::get<1>(*it2)->get_class()) {
              std::get<0>(*it2) = std::get<0>(*it);
            }
            // it is hard, it2 is hard
            // keep height when both are hard surfaces, add vw
            else if (std::get<1>(*it)->is_hard()) {
              if (std::get<1>(*it2)->is_hard()) {
                //- add a wall to the heighest feature
                if (std::get<1>(*it) > std::get<1>(*it2)) {
                  std::get<1>(*it)->add_vertical_wall();
                }
                else if (std::get<1>(*it2) > std::get<1>(*it)) {
                  std::get<1>(*it2)->add_vertical_wall();
                }
              }
              // it is hard, it2 is soft
              // set height of it2 to it
              else {
                std::get<0>(*it2) = std::get<0>(*it);
              }
            }
            // it is soft, it2 is hard
            // set height of it to it2
            else if (std::get<1>(*it2)->is_hard()) {
              std::get<0>(*it) = std::get<0>(*it2);
            }
          }
          // features are outside threshold jump edges, add vw
          else {
            //-- add a wall to the heighest feature, it2 is allways highest since zstart is sorted by height
            std::get<1>(*it2)->add_vertical_wall();
          }
        }
        //-- Average heights of soft features within the jumpedge threshold counted from the lowest feature or skip to the next hard feature
        // fnext is the last feature within threshold jump edge, average all soft in between
        if (it != fnext) {
          if (std::get<1>(*it)->is_hard() == false && std::get<1>(*fnext)->is_hard() == false) {
            int totalz = 0;
            int count = 0;
            for (std::vector< std::tuple< int, TopoFeature*, int, int > >::iterator it2 = it; it2 != fnext + 1; ++it2) {
              totalz += std::get<0>(*it2);
              count++;
            }
            totalz = totalz / count;
            for (std::vector< std::tuple< int, TopoFeature*, int, int > >::iterator it2 = it; it2 != fnext + 1; ++it2) {
              std::get<0>(*it2) = totalz;
            }
          }
          else if (std::get<1>(*it)->is_hard() == false) {
            // Adjust all intermediate soft features
            for (std::vector< std::tuple< int, TopoFeature*, int, int > >::iterator it2 = it; it2 != fnext; ++it2) {
              std::get<0>(*it2) = std::get<0>(*fnext);
            }
          }
          it = fnext;
        }
      }
    }

    //-- assign the adjusted heights and build the nc
    int tmph = -99999;
    for (auto& each : zstar) {
      std::get<1>(each)->set_vertex_elevation(std::get<2>(each), std::get<3>(each), std::get<0>(each));
      if (std::get<0>(each) != tmph) { //-- not to repeat the same height
        Point2 p = std::get<1>(each)->get_point2(std::get<2>(each), std::get<3>(each));
        _nc[gen_key_bucket(&p)].push_back(std::get<0>(each));
        tmph = std::get<0>(each);
      }
    }
  }
}

void Map3d::stitch_jumpedge(TopoFeature* f1, int ringi1, int pi1, TopoFeature* f2, int ringi2, int pi2) {
  Point2 p = f1->get_point2(ringi1, pi1);
  std::string key_bucket = gen_key_bucket(&p);
  int f1z = f1->get_vertex_elevation(ringi1, pi1);
  int f2z = f2->get_vertex_elevation(ringi2, pi2);

  //-- Buildings involved
  if ((f1->get_class() == BUILDING) || (f2->get_class() == BUILDING)) {
    if ((f1->get_class() == BUILDING) && (f2->get_class() == BUILDING)) {
      // add a wall to both buildings
      f1->add_vertical_wall();
      f2->add_vertical_wall();
      _nc[key_bucket].push_back(f1z);
      _nc[key_bucket].push_back(f2z);
      int f1base = dynamic_cast<Building*>(f1)->get_height_base();
      int f2base = dynamic_cast<Building*>(f2)->get_height_base();
      _nc[key_bucket].push_back(f1base);
      if (f1base != f2base) {
        _nc[key_bucket].push_back(f2base);
      }
    }
    else if (f1->get_class() == BUILDING) {
      if (f2->get_class() != WATER) {
        f2->set_vertex_elevation(ringi2, pi2, dynamic_cast<Building*>(f1)->get_height_base());
      }
      else {
        //- keep water flat, add the water height to the nc
        _nc[key_bucket].push_back(f2z);
      }
      //- expect a building to always be heighest adjacent feature
      f1->add_vertical_wall();
      _nc[key_bucket].push_back(f1z);
      _nc[key_bucket].push_back(dynamic_cast<Building*>(f1)->get_height_base());
    }
    else { //-- f2 is Building
      if (f1->get_class() != WATER) {
        f1->set_vertex_elevation(ringi1, pi1, dynamic_cast<Building*>(f2)->get_height_base());
      }
      else {
        //- keep water flat, add the water height to the nc
        _nc[key_bucket].push_back(f1z);
      }
      //- expect a building to always be heighest adjacent feature
      f2->add_vertical_wall();
      _nc[key_bucket].push_back(f2z);
      _nc[key_bucket].push_back(dynamic_cast<Building*>(f2)->get_height_base());
    }
  }
  //-- no Buildings involved
  else {
    bool bStitched = false;
    int deltaz = std::abs(f1z - f2z);
    if (deltaz < this->_threshold_jump_edges) {
      //-- handle same class, average them
      if (f1->get_class() == f2->get_class()) {
        int avgz = (f1->get_vertex_elevation(ringi1, pi1) + f2->get_vertex_elevation(ringi2, pi2)) / 2;
        f1->set_vertex_elevation(ringi1, pi1, avgz);
        f2->set_vertex_elevation(ringi2, pi2, avgz);
        _nc[key_bucket].push_back(avgz);
        bStitched = true;
      }
      if (f1->is_hard() == false) {
        f1->set_vertex_elevation(ringi1, pi1, f2z);
        bStitched = true;
      }
      else if (f2->is_hard() == false) {
        f2->set_vertex_elevation(ringi2, pi2, f1z);
        bStitched = true;
      }
    }
    //-- then vertical walls must be added: nc to highest
    if (bStitched == false) {
      //- add a wall to the heighest feature
      if (f1z > f2z) {
        f1->add_vertical_wall();
      }
      else if (f2z > f1z) {
        f2->add_vertical_wall();
      }
      _nc[key_bucket].push_back(f1z);
      _nc[key_bucket].push_back(f2z);
    }
  }
}

void Map3d::stitch_average(TopoFeature* f1, int ringi1, int pi1, TopoFeature* f2, int ringi2, int pi2) {
  int avgz = (f1->get_vertex_elevation(ringi1, pi1) + f2->get_vertex_elevation(ringi2, pi2)) / 2;
  f1->set_vertex_elevation(ringi1, pi1, avgz);
  f2->set_vertex_elevation(ringi2, pi2, avgz);
  Point2 p = f1->get_point2(ringi1, pi1);
  _nc[gen_key_bucket(&p)].push_back(avgz);
}

void Map3d::stitch_bridges() {
  std::vector<int> ringis, pis;
  for (auto& f : _lsFeatures) {
    if (f->get_class() == BRIDGE) {
      //-- 1. store all touching top level (adjacent + incident)
      std::vector<TopoFeature*>* lstouching = f->get_adjacent_features();

      //-- gather all rings
      std::vector<Ring2> rings;
      rings.push_back(f->get_Polygon2()->outer());
      for (Ring2& iring : f->get_Polygon2()->inners())
        rings.push_back(iring);

      int ringi = -1;
      for (Ring2& ring : rings) {
        ringi++;

        std::vector< std::pair<int, bool> > corners;
        for (int i = 0; i < ring.size(); i++) {
          Point2 p = f->get_point2(ringi, i);
          std::string key_bucket = gen_key_bucket(&p);
          std::vector<int> nc = _nc[key_bucket];
          std::sort(nc.begin(), nc.end());
          auto ncuIt = std::unique(nc.begin(), nc.end());
          int unique = std::distance(nc.begin(), ncuIt);

          if (unique > 0) {
            bool bridgeAdj = false;
            /* this can be 3 cases;
            1. location where two bridges and other object meet without height jump.
            2. location where one bridge meets multiple objects which should be a stitching error
            3. error in stitching adjacent objects
            */
            if (unique <= 1) { // only check for bridge adjacent if there is only 1 unique value in NC for performance
              for (auto& fadj : *lstouching) {
                ringis.clear();
                pis.clear();
                if (fadj->get_class() == BRIDGE && fadj->has_point2(ring[i], ringis, pis) == true) {
                  bridgeAdj = true;
                }
              }
            }
            /* Two cases which are seen as corners of the bridge object, either a height jump or where two bridge objects meet
            1. if bridge adjacent, use nc. This is the case where a bridge is split into multiple parts touching the adjacent object
            2. if 2 or more unique values in NC there is a height jump which should only occur at vw of bridge, skip values upto next occurence
            */
            if (bridgeAdj || unique > 1) {
              //TODO: use NC closest to height of previous vertex? Then is no choice to make and it might support multiple overlapping bridges?
              if (f->get_top_level() == false) {
                f->set_vertex_elevation(ringi, i, nc.front());
              }
              else {
                f->set_vertex_elevation(ringi, i, nc.back());
              }

              // if there are more then 1 unique height in NC we have a height jump.
              bool heightjump = false;
              if (unique > 1) {
                heightjump = true;
              }
              corners.push_back(std::make_pair(i, heightjump));
            }
          }
        }

        // Set height of vertices in between two corners
        for (int c = 0; c < corners.size(); c++) {
          // prepair vertices to loop for this stretch
          std::pair<int, bool> startCorner = corners[c];
          // set endCorner to first or corners if end of array is reached
          std::pair<int, bool> endCorner = corners.front();
          if (c + 1 < corners.size()) {
            endCorner = corners[c + 1];
          }
          int endCornerIdx = endCorner.first;

          std::vector<int> vertices;
          if (endCornerIdx < startCorner.first) {
            for (int i = 0; i < endCornerIdx; i++) {
              vertices.push_back(i);
            }
            endCornerIdx = ring.size();
          }
          for (int i = startCorner.first + 1; i < endCornerIdx; i++) {
            vertices.push_back(i);
          }

          for (int pi : vertices) {
            bool otherAdj = false;
            int stitchz = 0;
            std::vector< std::tuple<TopoFeature*, int, int> > star;
            for (auto& fadj : *lstouching) {
              ringis.clear();
              pis.clear();
              if (fadj->get_class() != BRIDGE && fadj->has_point2(ring[pi], ringis, pis) == true) {
                otherAdj = true;
                stitchz = fadj->get_vertex_elevation(ringis[0], pis[0]);
              }
            }

            if (!otherAdj) { //No other type of objects then bridges are adjacent
                             //This is empty stretch of points where only bridges connect, interpolate between prevCorner and nextCorner
              Point2 p = f->get_point2(ringi, pi);
              std::string key_bucket = gen_key_bucket(&p);
              int z = 0;
              //Check if height exists in NC thus is created by adjacent bridge
              if (_nc[key_bucket].empty()) {
                // Add height to NC for adjacent bridge
                // interpolate height between previous and next corner distance weighted
                // interpolate height between previous and next corner distance weighted
                z = interpolate_height(f, p, ringi, startCorner.first, ringi, endCorner.first);
                _nc[key_bucket].push_back(z);
              }
              else {
                z = _nc[key_bucket].front();
              }
              f->set_vertex_elevation(ringi, pi, z);
            }
            else if (startCorner.second && endCorner.second) { //startCorner.heightjump && endCorner.heightjump
                                                               //This is empty stretch of points at height jump and need to place VW
              Point2 p = f->get_point2(ringi, pi);
              std::string key_bucket = gen_key_bucket(&p);
              // interpolate height between previous and next corner distance weighted
              int z = interpolate_height(f, p, ringi, startCorner.first, ringi, endCorner.first);
              f->set_vertex_elevation(ringi, pi, z);
              // Add height to NC and add VW
              _nc[key_bucket].push_back(z);
              f->add_vertical_wall();
            }
            else {
              //are these the connected points? Only other case possible?
              //cases (not exaustive):
              //prevCorner.heightjump && this.otherAdj
              //nextCorner.heightjump && this.otherAdj
              // Stitch to acjacent feature
              f->set_vertex_elevation(ringi, pi, stitchz);
            }
          }
        }
      }
    }
  }
}

int Map3d::interpolate_height(TopoFeature* f, const Point2 &p, int prevringi, int prevpi, int nextringi, int nextpi) {
  // interpolate height between previous and next corner distance weighted
  double dprev = distance(p, f->get_point2(prevringi, prevpi));
  double dnext = distance(p, f->get_point2(nextringi, nextpi));
  double dtotal = dprev + dnext;
  return int((dnext / dtotal) * f->get_vertex_elevation(prevringi, prevpi) + (dprev / dtotal) * f->get_vertex_elevation(nextringi, nextpi));
}

void Map3d::add_allowed_las_class(AllowedLASTopo c, int i) {
  _las_classes_allowed[c].insert(i);
}
