/*
  3dfier: takes 2D GIS datasets and "3dfies" to create 3D city models.

  Copyright (C) 2015-2020 3D geoinformation research group, TU Delft

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

/**
 * Map3D contains all configuration settings and functions to create the 3D model.
 * It contains functions to read polygons, points, threedfy, construct CDT and 
 * create all output formats.
 */

#include "Map3d.h"

Map3d::Map3d() {
  OGRRegisterAll();
  _building_heightref_roof = 0.9;
  _building_heightref_ground = 0.1;
  _building_triangulate = true;
  _building_lod = 1;
  _building_include_floor = false;
  _building_inner_walls = false;
  _terrain_simplification = 0;
  _forest_simplification = 0;
  _terrain_simplification_tinsimp = 0.0;
  _forest_simplification_tinsimp = 0.0;
  _terrain_innerbuffer = 0.0;
  _forest_innerbuffer = 0.0;
  _water_heightref = 0.1;
  _road_heightref = 0.5;
  _road_filter_outliers = true;
  _road_flatten = true;
  _separation_heightref = 0.8;
  _bridge_heightref = 0.5;
  _radius_vertex_elevation = 1.0;
  _building_radius_vertex_elevation = 3.0;
  _threshold_jump_edges = 50;
  _threshold_bridge_jump_edges = 50;
  _requestedExtent = Box2(Point2(0, 0), Point2(0, 0));
  _bbox = Box2(Point2(9999999, 9999999), Point2(-9999999, -9999999));
  _minxradius = 9999999;
  _maxxradius = 9999999;
  _minyradius = -9999999;
  _maxyradius = -9999999;
  _max_angle_curvepolygon = 0;
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

void Map3d::set_threshold_bridge_jump_edges(float threshold) {
  _threshold_bridge_jump_edges = int(threshold * 100);
}

void Map3d::set_building_include_floor(bool include) {
  _building_include_floor = include;
}

void Map3d::set_building_triangulate(bool triangulate) {
  _building_triangulate = triangulate;
}

void Map3d::set_building_inner_walls(bool inner_walls) {
  _building_inner_walls = inner_walls;
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

void Map3d::set_road_filter_outliers(bool filter) {
  _road_filter_outliers = filter;
}

void Map3d::set_road_flatten(bool flatten) {
  _road_flatten = flatten;
}

void Map3d::set_separation_heightref(float h) {
  _separation_heightref = h;
}

void Map3d::set_bridge_heightref(float h) {
  _bridge_heightref = h;
}

void Map3d::set_bridge_flatten(bool flatten) {
  _bridge_flatten = flatten;
}

void Map3d::set_requested_extent(double xmin, double ymin, double xmax, double ymax) {
  _requestedExtent = Box2(Point2(xmin, ymin), Point2(xmax, ymax));
}

void Map3d::set_max_angle_curvepolygon(double max_angle) {
  _max_angle_curvepolygon = max_angle;
}

Box2 Map3d::get_bbox() {
  return _bbox;
}

bool Map3d::check_bounds(const double xmin, const double xmax, const double ymin, const double ymax) {
  if ((xmin < _maxxradius || xmax > _minxradius) &&
    (ymin < _maxyradius || ymax > _minyradius)) {
    return true;
  }
  return false;
}

bool Map3d::get_cityjson(std::wostream& of) {
  nlohmann::json j;
  j["type"] = "CityJSON";
  j["version"] = "1.0";
  j["metadata"] = {};
  double b[] = {bg::get<bg::min_corner, 0>(_bbox),
                bg::get<bg::min_corner, 1>(_bbox), 
                0,
                bg::get<bg::max_corner, 0>(_bbox),
                bg::get<bg::max_corner, 1>(_bbox), 
                0};
  j["metadata"]["geographicalExtent"] = b;
  j["metadata"]["referenceSystem"] = "urn:ogc:def:crs:EPSG::7415";
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
  of << j.dump() << std::endl;
  return true;
}

void Map3d::get_citygml(std::wostream& of) {
  create_citygml_header(of);
  for (auto& f : _lsFeatures) {
    f->get_citygml(of);
    of << "\n";
  }
  of << "</CityModel>\n";
}

void Map3d::get_citygml_multifile(std::string ofname) {
  std::unordered_map<std::string, std::wofstream*> ofs;

  for (auto& f : _lsFeatures) {
    std::string filename = ofname + f->get_layername() + ".gml";
    if (ofs.find(filename) == ofs.end()) {
      std::wofstream* of = new std::wofstream();
      of->open(filename);
      ofs.emplace(filename, of);
      create_citygml_header(*ofs[filename]);
    }
    f->get_citygml(*ofs[filename]);
    *ofs[filename] << "\n";
  }
  for (auto it = ofs.begin(); it != ofs.end(); it++) {
    std::wofstream& of = *(it->second);
    of << "</CityModel>\n";
    of.close();
  }
}

void Map3d::get_citygml_imgeo(std::wostream& of) {
  create_citygml_imgeo_header(of);
  for (auto& f : _lsFeatures) {
    f->get_citygml_imgeo(of);
    of << "\n";
  }
  of << "</CityModel>\n";
}

void Map3d::get_citygml_imgeo_multifile(std::string ofname) {
  std::unordered_map<std::string, std::wofstream*> ofs;

  for (auto& f : _lsFeatures) {
    std::string filename = ofname + f->get_layername() + ".gml";
    if (ofs.find(filename) == ofs.end()) {
      std::wofstream* of = new std::wofstream();
      of->open(filename);
      ofs.emplace(filename, of);
      create_citygml_imgeo_header(*ofs[filename]);
    }
    f->get_citygml_imgeo(*ofs[filename]);
    *ofs[filename] << "\n";
  }
  for (auto it = ofs.begin(); it != ofs.end(); it++) {
    std::wofstream& of = *(it->second);
    of << "</CityModel>\n";
    of.close();
  }
}

void Map3d::create_citygml_header(std::wostream& of) {
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

void Map3d::create_citygml_imgeo_header(std::wostream& of) {
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

void Map3d::get_csv_buildings(std::wostream& of) {
  of << "id,roof,ground\n";
  for (auto& p : _lsFeatures) {
    if (p->get_class() == BUILDING) {
      Building* b = dynamic_cast<Building*>(p);
      b->get_csv(of);
    }
  }
}

void Map3d::get_csv_buildings_all_elevation_points(std::wostream& of) {
  of << "id,allzvalues" << std::endl;
  for (auto& p : _lsFeatures) {
    if (p->get_class() == BUILDING) {
      Building* b = dynamic_cast<Building*>(p);
      of << b->get_id() << ",";
      of << b->get_all_z_values();
      of << std::endl;
    }
  }
}

void Map3d::get_csv_buildings_multiple_heights(std::wostream& of) {
  //-- ground heights
  std::vector<float> gpercentiles = {0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f};
  std::vector<float> rpercentiles = {0.0f, 0.1f, 0.25f, 0.5f, 0.75f, 0.9f, 0.95f, 0.99f};
  of << std::setprecision(2) << std::fixed;
  of << "id";
  for (auto& each : gpercentiles)
    of << ",ground-" << each;
  for (auto& each : rpercentiles)
    of << ",roof-" << each;
  of << std::endl;
  for (auto& p : _lsFeatures) {
    if (p->get_class() == BUILDING) {
      Building* b = dynamic_cast<Building*>(p);
      of << b->get_id();
      for (auto& each : gpercentiles) {
        int h = b->get_height_ground_at_percentile(each);
        of << "," << float(h)/100;
      }
      for (auto& each : rpercentiles) {
        int h = b->get_height_roof_at_percentile(each);
        of << "," << float(h)/100;
      }
      of << std::endl;
    }
  }
}

void Map3d::get_obj_per_feature(std::wostream& of) {
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

void Map3d::get_obj_per_class(std::wostream& of) {
  std::unordered_map< std::string, unsigned long > dPts;
  std::string fs;
  for (int c = 0; c < 7; c++) {
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

void Map3d::get_stl(std::wostream& of) {
  std::unordered_map< std::string, unsigned long > dPts;
  std::string fs;
  
  for (int c = 0; c < 7; c++) {
    for (auto& p : _lsFeatures) {
      if (p->get_class() == c) {
        if (p->get_class() == BUILDING) {
          Building* b = dynamic_cast<Building*>(p);
          b->get_stl(dPts, _building_lod, b->get_mtl(), fs);
        }
        else {
          p->get_stl(dPts, p->get_mtl(), fs);
        }
      }
    }
  }

  //-- sort the points in the map: simpler to copy to a vector
//  std::vector<std::string> thepts;
//  thepts.resize(dPts.size());
//  for (auto& p : dPts)
//    thepts[p.second - 1] = p.first;
//  dPts.clear();

  of << "mtllib ./3dfier.mtl" << "\n";
//  for (auto& p : thepts) {
//    of << "v " << p << "\n";
//  }
  of << fs << std::endl;
}


bool Map3d::get_postgis_output(std::string connstr, bool pdok, bool citygml) {
#if GDAL_VERSION_MAJOR < 2
  std::cerr << "ERROR: cannot write MultiPolygonZ files with GDAL < 2.0.\n";
  return false;
#else
  if (GDALGetDriverCount() == 0)
    GDALAllRegister();
  GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("PostgreSQL");
  GDALDataset* dataSource = driver->Create(connstr.c_str(), 0, 0, 0, GDT_Unknown, NULL);
  if (dataSource == NULL) {
    std::cerr << "Starting database connection failed.\n";
    return false;
  }
  if (dataSource->StartTransaction() != OGRERR_NONE) {
    std::cerr << "Starting database transaction failed.\n";
    return false;
  }

  std::unordered_map<std::string, OGRLayer*> layers;
  // create and write layers first
  for (auto& f : _lsFeatures) {
    std::string layername = f->get_layername();
    if (layers.find(layername) == layers.end()) {
      AttributeMap &attributes = f->get_attributes();
      if (pdok) {
        //Add additional attribute to list for layer creation
        attributes["xml"] = std::make_pair(OFTString, "");
      }
      OGRLayer *layer = create_gdal_layer(driver, dataSource, connstr, layername, attributes, f->get_class() == BUILDING);
      if (layer == NULL) {
        std::cerr << "ERROR: Cannot open database '" + connstr + "' for writing" << std::endl;
        dataSource->RollbackTransaction();
        GDALClose(dataSource);
        GDALClose(driver);
        return false;
      }
      layers.emplace(layername, layer);
    }
  }
  if (dataSource->CommitTransaction() != OGRERR_NONE) {
    std::cerr << "Writing to database failed.\n";
    return false;
  }
  if (dataSource->StartTransaction() != OGRERR_NONE) {
    std::cerr << "Starting database transaction failed.\n";
    return false;
  }

  // create and write features to layers
  int i = 1;
  for (auto& f : _lsFeatures) {
    std::string layername = f->get_layername();
    AttributeMap extraAttribute = AttributeMap();
    
    if (pdok) {
      //Add additional attribute describing CityGML of feature
      std::wstring_convert<codecvt<wchar_t, char, std::mbstate_t>>converter;
      std::wstringstream ss;
      ss << std::fixed << std::setprecision(3);
      if (citygml) {
        f->get_citygml(ss);
      }
      else {
        f->get_citygml_imgeo(ss);
      }
      std::string gmlAttribute = converter.to_bytes(ss.str());
      ss.clear();
      extraAttribute["xml"] = std::make_pair(OFTString, gmlAttribute);
    }
    if (!f->get_shape(layers[layername], true, extraAttribute)) {
      return false;
    }
    if (i % 1000 == 0) {
      if (dataSource->CommitTransaction() != OGRERR_NONE) {
        std::cerr << "Writing to database failed.\n";
        return false;
      }
      if (dataSource->StartTransaction() != OGRERR_NONE) {
        std::cerr << "Starting database transaction failed.\n";
        return false;
      }
    }
    i++;
  }
  if (dataSource->CommitTransaction() != OGRERR_NONE) {
    std::cerr << "Writing to database failed.\n";
    return false;
  }
  GDALClose(dataSource);
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
    OGRLayer *layer = create_gdal_layer(driver, NULL, filename, "my3dmap", AttributeMap(), true);
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
        OGRLayer *layer = create_gdal_layer(driver, NULL, tmpFilename, layername, f->get_attributes(), f->get_class() == BUILDING);
        if (layer == NULL) {
          std::cerr << "ERROR: Cannot open file '" + filename + "' for writing" << std::endl;
          close_gdal_resources(driver, layers);
          return false;
        }
        layers.emplace(layername, layer);
      }
      if (!f->get_shape(layers[layername], true)) {
        return false;
      }
    }
    close_gdal_resources(driver, layers);
  }
  return true;
#endif
}

#if GDAL_VERSION_MAJOR >= 2
void Map3d::close_gdal_resources(GDALDriver* driver, std::unordered_map<std::string, OGRLayer*> layers) {
  for (auto& layer : layers) {
    GDALClose(layer.second);
  }
  GDALClose(driver);
}
#endif

#if GDAL_VERSION_MAJOR >= 2
OGRLayer* Map3d::create_gdal_layer(GDALDriver* driver, GDALDataset* dataSource, std::string filename, std::string layername, AttributeMap attributes, bool addHeightAttributes) {
  if (dataSource == NULL) {
    dataSource = driver->Create(filename.c_str(), 0, 0, 0, GDT_Unknown, NULL);
  }

  if (dataSource == NULL) {
    std::cerr << "ERROR: could not open file, skipping it.\n";
    return NULL;
  }
  OGRLayer *layer = dataSource->GetLayerByName(layername.c_str());
  if (layer == NULL) {
    OGRSpatialReference* sr = new OGRSpatialReference();
    sr->importFromEPSG(7415);
    layer = dataSource->CreateLayer(layername.c_str(), sr, OGR_GT_SetZ(wkbMultiPolygon), NULL);

    OGRFieldDefn oField("3df_id", OFTString);
    if (layer->CreateField(&oField) != OGRERR_NONE) {
      std::cerr << "Creating 3df_id field failed.\n";
      return NULL;
    }
    OGRFieldDefn oField2("3df_class", OFTString);
    if (layer->CreateField(&oField2) != OGRERR_NONE) {
      std::cerr << "Creating 3df_class field failed.\n";
      return NULL;
    }
    if (addHeightAttributes) {
      OGRFieldDefn oField3("baseheight", OFTReal);
      if (layer->CreateField(&oField3) != OGRERR_NONE) {
        std::cerr << "Creating floorheight field failed.\n";
        return NULL;
      }
      OGRFieldDefn oField4("RoofHeight", OFTReal);
      if (layer->CreateField(&oField4) != OGRERR_NONE) {
        std::cerr << "Creating roofheight field failed.\n";
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

unsigned long Map3d::get_num_polygons() {
  return _lsFeatures.size();
}

const std::vector<TopoFeature*>& Map3d::get_polygons3d() {
  return _lsFeatures;
}

/**
 * process a LAS/LAZ point
 * search rtrees for intersecting features
 * check if points classification is allowed for feature and add point to feature
 */
void Map3d::add_elevation_point(LASpoint const& laspt) {
  //-- only process last returns; 
  //-- although perhaps not smart for vegetation/forest in the future
  //-- TODO: always ignore the non-last-return points?
  if (laspt.return_number != laspt.number_of_returns)
    return;

  std::vector<PairIndexed> re;
  float x = laspt.get_x();
  float y = laspt.get_y();
  Point2 minp(x - _radius_vertex_elevation, y - _radius_vertex_elevation);
  Point2 maxp(x + _radius_vertex_elevation, y + _radius_vertex_elevation);
  Box2 querybox(minp, maxp);
  _rtree.query(bgi::intersects(querybox), std::back_inserter(re));
  minp = Point2(x - _building_radius_vertex_elevation, y - _building_radius_vertex_elevation);
  maxp = Point2(x + _building_radius_vertex_elevation, y + _building_radius_vertex_elevation);
  querybox = Box2(minp, maxp);
  _rtree_buildings.query(bgi::intersects(querybox), std::back_inserter(re));

  for (auto& v : re) {
    TopoFeature* f = v.second;
    float radius = _radius_vertex_elevation;

    int c = (int)laspt.classification;
    bool bInsert = false;
    bool bWithin = false;
    if (f->get_class() == BUILDING) {
      bInsert = true;
      radius = _building_radius_vertex_elevation;
    }
    else if (f->get_class() == TERRAIN) {
      if (_las_classes_allowed[LAS_TERRAIN].empty() || _las_classes_allowed[LAS_TERRAIN].count(c) > 0) {
        bInsert = true;
      }
      if (_las_classes_allowed_within[LAS_TERRAIN].count(c) > 0) {
        bInsert = true;
        bWithin = true;
      }
    }
    else if (f->get_class() == FOREST) {
      if (_las_classes_allowed[LAS_FOREST].empty() || _las_classes_allowed[LAS_FOREST].count(c) > 0) {
        bInsert = true;
      }
      if (_las_classes_allowed_within[LAS_FOREST].count(c) > 0) {
        bInsert = true;
        bWithin = true;
      }
    }
    else if (f->get_class() == ROAD) {
      if (_las_classes_allowed[LAS_ROAD].empty() || _las_classes_allowed[LAS_ROAD].count(c) > 0) {
        bInsert = true;
      }
      if (_las_classes_allowed_within[LAS_ROAD].count(c) > 0) {
        bInsert = true;
        bWithin = true;
      }
    }
    else if (f->get_class() == WATER) {
      if (_las_classes_allowed[LAS_WATER].empty() || _las_classes_allowed[LAS_WATER].count(c) > 0) {
        bInsert = true;
      }
      if (_las_classes_allowed_within[LAS_WATER].count(c) > 0) {
        bInsert = true;
        bWithin = true;
      }
    }
    else if (f->get_class() == SEPARATION) {
      if (_las_classes_allowed[LAS_SEPARATION].empty() || _las_classes_allowed[LAS_SEPARATION].count(c) > 0) {
        bInsert = true;
      }
      if (_las_classes_allowed_within[LAS_SEPARATION].count(c) > 0) {
        bInsert = true;
        bWithin = true;
      }
    }
    else if (f->get_class() == BRIDGE) {
      if (_las_classes_allowed[LAS_BRIDGE].empty() || _las_classes_allowed[LAS_BRIDGE].count(c) > 0) {
        bInsert = true;
      }
      if (_las_classes_allowed_within[LAS_BRIDGE].count(c) > 0) {
        bInsert = true;
        bWithin = true;
      }
    }
    if (bInsert == true) { //-- only insert if in the allowed LAS classes
      Point2 p(x, y);
      f->add_elevation_point(p, laspt.get_z(), radius, c, bWithin);
    }
  }
}

void Map3d::cleanup_elevations() {
  for (auto& f : _lsFeatures) {
    f->cleanup_elevations();
  }
}

/**
 * algorithm to create the 3D model from the input data
 * 
 * 1. lift polygon vertices to calculated height from point cloud
 * 2. stitch vertices to solve for vertical gaps and height jumps
 * 3. fix bow ties created by wrong stitching
 * 4. create vertical walls and building walls
*/
bool Map3d::threeDfy(bool stitching) {

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
        // make values in nc unique
        nc.second.erase(unique(nc.second.begin(), nc.second.end()), nc.second.end());
      }
      for (auto& nc : _nc_building_walls) {
        std::sort(nc.second.begin(), nc.second.end());
        // make values in nc unique
        nc.second.erase(unique(nc.second.begin(), nc.second.end()), nc.second.end());
      }

      std::clog << "=====  /BOWTIES =====\n";
      for (auto& f : _lsFeatures) {
        if (f->has_vertical_walls()) {
          f->fix_bowtie();
        }
      }
      std::clog << "=====  BOWTIES/ =====\n";

      std::clog << "=====  /VERTICAL WALLS =====\n";
      for (auto& f : _lsFeatures) {
        if (f->get_class() == BUILDING) {
          Building* b = dynamic_cast<Building*>(f);
          b->construct_building_walls(_nc_building_walls);
        }
        else if (f->has_vertical_walls()) {
          f->construct_vertical_walls(_nc);
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

/**
 * build the Constrained Delaunay Triangulation for each TopoFeature
 */
bool Map3d::construct_CDT() {
  std::clog << "=====  /CDT =====\n";
  for (auto& p : _lsFeatures) {
    try {
      p->buildCDT();
    }
    catch (std::exception e) {
      std::cerr << std::endl << "CDT failed for object \'" << p->get_id() << "\' (class " << p->get_class() << ") with error: " << e.what() << std::endl;
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

/**
 * create rtrees, one for building and one for other objects
 * calculate a single bounding box from both trees
 */
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
  
  double radius = std::max(_radius_vertex_elevation, _building_radius_vertex_elevation);
  _minxradius = std::min(bg::get<bg::min_corner, 0>(_rtree.bounds()), bg::get<bg::min_corner, 0>(_rtree_buildings.bounds())) - radius;
  _maxxradius = std::min(bg::get<bg::min_corner, 1>(_rtree.bounds()), bg::get<bg::min_corner, 1>(_rtree_buildings.bounds())) + radius;
  _minyradius = std::max(bg::get<bg::max_corner, 0>(_rtree.bounds()), bg::get<bg::max_corner, 0>(_rtree_buildings.bounds())) - radius;
  _maxyradius = std::max(bg::get<bg::max_corner, 1>(_rtree.bounds()), bg::get<bg::max_corner, 1>(_rtree_buildings.bounds())) + radius;
  return true;
}

/**
 * read a polygon file
 * setup the GDAL driver, datasource and read layers
 */
bool Map3d::add_polygons_files(std::vector<PolygonFile> &files) {
#if GDAL_VERSION_MAJOR < 2
  if (OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount() == 0)
    OGRRegisterAll();
#else
  if (GDALGetDriverCount() == 0)
    GDALAllRegister();
#endif

  for (auto file = files.begin(); file != files.end(); ++file) {
    std::string logstring = "Reading input dataset: " + file->filename;
    if (strncmp(file->filename.c_str(), "PG:", strlen("PG:")) == 0) {
      logstring = "Opening PostgreSQL database connection.";
    }
    std::clog << logstring << std::endl;

#if GDAL_VERSION_MAJOR < 2
    OGRDataSource *dataSource = OGRSFDriverRegistrar::Open(file->filename.c_str(), false);
#else
    GDALDataset *dataSource = (GDALDataset*)GDALOpenEx(file->filename.c_str(), GDAL_OF_READONLY | GDAL_OF_VECTOR, NULL, NULL, NULL);
#endif

    if (dataSource == NULL) {
      std::cerr << "\tERROR: " << logstring << std::endl;
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

/**
 * read a polygon layer
 * read id and relative height attributes and check if polygon is within max extent
 * Split MultiPolygons/MultiSurface and stroke CurvedPolygons
 */
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
    if (strlen(heightfield) == 0) {
      std::clog << "Using all polygons in layer '" << l.first << "'.\n";
    }
    else if (dataLayer->FindFieldIndex(heightfield, false) == -1) {
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
    OGREnvelope extent = OGREnvelope();
    if (boost::geometry::area(_requestedExtent) > 0) {
      extent.MinX = bg::get<bg::min_corner, 0>(_requestedExtent);
      extent.MaxX = bg::get<bg::max_corner, 0>(_requestedExtent);
      extent.MinY = bg::get<bg::min_corner, 1>(_requestedExtent);
      extent.MaxY = bg::get<bg::max_corner, 1>(_requestedExtent);
      useRequestedExtent = true;
    }

    int numSplitMulti = 0;
    int numSplitPoly = 0;
    int numCurvePoly = 0;
    while ((f = dataLayer->GetNextFeature()) != NULL) {
      OGRGeometry *geometry = f->GetGeometryRef();
      if (!geometry->IsValid()) {
        std::cerr << "Geometry invalid: " << f->GetFieldAsString(idfield) << std::endl;
      }
      OGREnvelope env;
      if (useRequestedExtent) {
        geometry->getEnvelope(&env);
      }

      //-- add the polygon when no extent is given or if the boundinx box of the polygon is within the extent
      if (!useRequestedExtent || extent.Intersects(env)) {
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
        case wkbCurvePolygon: {
          OGRCurvePolygon* curve_polygon = geometry->toCurvePolygon();
          OGRPolygon* polygon = curve_polygon->CurvePolyToPoly(_max_angle_curvepolygon);
          f->SetGeometry(polygon);
          extract_feature(f, layerName, idfield, heightfield, l.second, multiple_heights);
          numCurvePoly++;
          break;
        }
        case wkbMultiSurface:
        {
          OGRMultiSurface* multisurface = (OGRMultiSurface*)geometry;
          int numGeom = multisurface->getNumGeometries();
          if (numGeom >= 1) {
            for (int i = 0; i < numGeom; i++) {
              OGRFeature* cf = f->Clone();
              if (numGeom > 1) {
                std::string idString = (std::string)f->GetFieldAsString(idfield) + "-" + std::to_string(i);
                cf->SetField(idfield, idString.c_str());
              }
              OGRPolygon* polygon = multisurface->getGeometryRef(i)->toCurvePolygon()->CurvePolyToPoly(_max_angle_curvepolygon);
              cf->SetGeometry(polygon);
              extract_feature(cf, layerName, idfield, heightfield, l.second, multiple_heights);
            }
            numSplitMulti++;
            numSplitPoly += numGeom;
            numCurvePoly += numGeom;
          }
          break;
        }
        default: {
          std::cerr << "Geometry type is unsupported: " << geometry->getGeometryName() << std::endl;
          continue;
        }
        }
      }
      OGRFeature::DestroyFeature(f);
    }
    if (numSplitMulti > 0) {
      std::clog << "\tSplit " << numSplitMulti << " MultiPolygon(s) into " << numSplitPoly << " Polygon(s)\n";
    }
    if (numCurvePoly > 0) {
      if (_max_angle_curvepolygon == 0.0) {
        std::clog << "\tStroked " << numCurvePoly << " CurvePolygon(s) with a maximum angle of 4.0 degrees (default value)\n";
      }
      else {
        std::clog << "\tStroked " << numCurvePoly << " CurvePolygon(s) with a maximum angle of " << _max_angle_curvepolygon << "degrees\n";
      }
    }
    wentgood = true;
  }
  return wentgood;
}

/**
 * extract the GDAL feature
 * force polygon to 2D and read attributes
 * create a TopoFeature from the GDAL feature
 */
void Map3d::extract_feature(OGRFeature *f, std::string layername, const char *idfield, const char *heightfield, std::string layertype, bool multiple_heights) {
  char *wkt;
  OGRGeometry *geom = f->GetGeometryRef();
  geom->flattenTo2D();
  geom->exportToWkt(&wkt);
  AttributeMap attributes;
  int attributeCount = f->GetFieldCount();
  std::string id = f->GetFieldAsString(idfield);
  for (int i = 0; i < attributeCount; i++) {
    attributes[boost::locale::to_lower(f->GetFieldDefnRef(i)->GetNameRef())] = std::make_pair(f->GetFieldDefnRef(i)->GetType(), f->GetFieldAsString(i));
  }
  if (layertype == "Building") {
    Building* p3 = new Building(wkt, layername, attributes, id, _building_heightref_roof, _building_heightref_ground, _building_triangulate, _building_include_floor, _building_inner_walls);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Terrain") {
    Terrain* p3 = new Terrain(wkt, layername, attributes, id, this->_terrain_simplification, this->_terrain_simplification_tinsimp, this->_terrain_innerbuffer);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Forest") {
    Forest* p3 = new Forest(wkt, layername, attributes, id, this->_forest_simplification, this->_forest_simplification_tinsimp, this->_forest_innerbuffer);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Water") {
    Water* p3 = new Water(wkt, layername, attributes, id, this->_water_heightref);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Road") {
    Road* p3 = new Road(wkt, layername, attributes, id, this->_road_heightref, this->_road_filter_outliers, this->_road_flatten);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Separation") {
    Separation* p3 = new Separation(wkt, layername, attributes, id, this->_separation_heightref);
    _lsFeatures.push_back(p3);
  }
  else if (layertype == "Bridge/Overpass") {
    Bridge* p3 = new Bridge(wkt, layername, attributes, id, this->_bridge_heightref, _bridge_flatten);
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
  CPLFree(wkt);
}

/**
 * read a LAS/LAZ file
 * read header, get extent and check if file intersects with Map3D bounding box
 * apply filters set in configuration (class, thinning, extent)
 * check if point intersects with Map3D bounding box
 */
bool Map3d::add_las_file(PointFile pointFile) {
  std::clog << "Reading LAS/LAZ file: " << pointFile.filename << std::endl;

  LASreadOpener lasreadopener;
  lasreadopener.set_file_name(pointFile.filename.c_str());
  //-- set to compute bounding box
  lasreadopener.set_populate_header(true);
  LASreader* lasreader = lasreadopener.open();

  try {
    //-- check if file is open
    if (lasreader == 0) {
      std::cerr << "\tERROR: could not open file: " << pointFile.filename << std::endl;
      return false;
    }
    LASheader header = lasreader->header;

    if (check_bounds(header.min_x, header.max_x, header.min_y, header.max_y)) {
      //-- LAS classes to omit
      std::vector<int> lasomits;
      for (int i : pointFile.lasomits) {
        lasomits.push_back(i);
      }

      //-- read each point 1-by-1
      uint32_t pointCount = header.number_of_point_records;

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
      while (lasreader->read_point()) {
        LASpoint const& p = lasreader->point;
        //-- set the thinning filter
        if (i % pointFile.thinning == 0) {
          //-- set the classification filter
          if (std::find(lasomits.begin(), lasomits.end(), (int)p.classification) == lasomits.end()) {
            //-- set the bounds filter
            if (check_bounds(p.X, p.X, p.Y, p.Y)) {
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
    else {
      std::clog << "\tskipping file, bounds do not intersect polygon extent\n";
    }
    lasreader->close();
  }
  catch (std::exception e) {
    std::cerr << std::endl << e.what() << std::endl;
    lasreader->close();
    return false;
  }
  return true;
}

/**
 * query rtrees and iterate results to find adjacent features
 */
void Map3d::collect_adjacent_features(TopoFeature* f) {
  std::vector<PairIndexed> re;
  Box2 b = f->get_bbox2d();
  _rtree.query(bgi::satisfies([&](PairIndexed const& v) {return bg::distance(v.first, b) < TOPODIST; }), std::back_inserter(re));
  _rtree_buildings.query(bgi::satisfies([&](PairIndexed const& v) {return bg::distance(v.first, b) < TOPODIST; }), std::back_inserter(re));
  for (auto& each : re) {
    TopoFeature* fadj = each.second;
    if (f != fadj && f->adjacent(*(fadj->get_Polygon2()))){
      f->add_adjacent_feature(fadj);
    }
  }
}

/**
 * main function for stitching all features
 * adjacent vertices with different heights from lifting are 
 * changed based on adjacency rules so the output model will 
 * not have gaps and height jumps
 */ 
void Map3d::stitch_lifted_features() {
  std::vector<int> ringis, pis;
  for (auto& f : _lsFeatures) {
    if (f->get_class() != BRIDGE) {
      //-- gather all rings
      std::vector<Ring2> therings;
      Polygon2* poly = f->get_Polygon2();
      therings.push_back(poly->outer());
      for (Ring2& iring : poly->inners())
        therings.push_back(iring);

      int ringi = -1;
      for (Ring2& ring : therings) {
        ringi++;
        //-- 1. store all touching top level (adjacent + incident)
        std::vector<TopoFeature*>* lstouching = f->get_adjacent_features();
        //-- 2. build the node-column for each vertex
        for (int i = 0; i < ring.size(); i++) {
          std::vector< std::tuple<TopoFeature*, int, int> > star;
          bool toprocess = false;
          for (auto& fadj : *lstouching) {
            ringis.clear();
            pis.clear();
            if (fadj->has_point2(ring[i], ringis, pis) == true) {
              for (int k = 0; k < ringis.size(); k++) {
                toprocess = true;
                star.push_back(std::make_tuple(fadj, ringis[k], pis[k]));
              }
            }
          }
          if (toprocess == true) {
            this->stitch_one_vertex(f, ringi, i, star);
          }
          else if (f->get_class() == BUILDING) {
            Point2 tmp = f->get_point2(ringi, i);
            std::string key_bucket = gen_key_bucket(&tmp);
            int z = dynamic_cast<Building*>(f)->get_height_base();
            _nc_building_walls[key_bucket].push_back(z);
            z = f->get_vertex_elevation(ringi, i);
            _nc_building_walls[key_bucket].push_back(z);
          }
        }
      }
    }
  }
}

/**
 * stitch multiple heights at a single vertex
 * rules depend on object types containing this vertex and their height differences
 */
void Map3d::stitch_one_vertex(TopoFeature* f, int ringi, int pi, std::vector< std::tuple<TopoFeature*, int, int> >& star) {
  //-- get p and key_bucket once and check if nc location is empty
  Point2 p = f->get_point2(ringi, pi);
  std::string key_bucket = gen_key_bucket(&p);
  if (_nc.find(key_bucket) == _nc.end() && _nc_building_walls.find(key_bucket) == _nc_building_walls.end()) {
    //-- degree of vertex == 2
    if (star.size() == 1) {
      if (std::get<0>(star[0])->get_class() != BRIDGE) {
        TopoFeature* fadj = std::get<0>(star[0]);
        //-- if not building or both soft, then average the heights
        if (f->get_class() != BUILDING && fadj->get_class() != BUILDING && (f->is_hard() == false && fadj->is_hard() == false)) {
          stitch_average(f, ringi, pi, fadj, std::get<1>(star[0]), std::get<2>(star[0]));
        }
        else { //-- there might be a heightjump here so stitch using the jumpedge settings
          stitch_jumpedge(f, ringi, pi, fadj, std::get<1>(star[0]), std::get<2>(star[0]));
        }
      }
    }
    //-- degree of vertex >= 3: more complex cases
    else if (star.size() > 1) {
      //-- collect all elevations
      std::vector< std::tuple< int, TopoFeature*, int, int > > zstar;
      //-- add own elevation to the zstar
      zstar.push_back(std::make_tuple(
        f->get_vertex_elevation(ringi, pi),
        f,
        ringi,
        pi));
      //-- add heights of adjacent features to the zstar
      for (auto& fadj : star) {
        //-- adjacent feature not Bridge or Building so add the height to the zstar
        if (std::get<0>(fadj)->get_class() != BRIDGE && std::get<0>(fadj)->get_class() != BUILDING) {
          zstar.push_back(std::make_tuple(
            std::get<0>(fadj)->get_vertex_elevation(std::get<1>(fadj), std::get<2>(fadj)),
            std::get<0>(fadj),
            std::get<1>(fadj),
            std::get<2>(fadj)));
        } //-- adjacent feature is a building so add the floor height to the zstar
        else if (std::get<0>(fadj)->get_class() == BUILDING) {
          zstar.push_back(std::make_tuple(
            dynamic_cast<Building*>(std::get<0>(fadj))->get_height_base(),
            std::get<0>(fadj),
            std::get<1>(fadj),
            std::get<2>(fadj)));
        }
        // This it for adjacent objects at the corners of a bridge where the adjacent features need extra VW at the height jump.
        else {
          f->add_vertical_wall();
        }
      }

      //-- sort low-high based on heights (get<0>)
      std::sort(zstar.begin(), zstar.end(),
        [](std::tuple<int, TopoFeature*, int, int> const &t1, std::tuple<int, TopoFeature*, int, int> const &t2) {
        return std::get<0>(t1) < std::get<0>(t2);
      });

      //-- Identify buildings and water to handle specific cases
      int building = -1;
      int water = -1;
      std::vector<int> buildings;
      for (int i = 0; i < zstar.size(); i++) {
        TopoClass topoClass = std::get<1>(zstar[i])->get_class();
        if (topoClass == BUILDING) {
          //-- store building indices
          buildings.push_back(i);
          //-- set building to the one with the highest base
          if (building == -1 || dynamic_cast<Building*>(std::get<1>(zstar[i]))->get_height_base() > dynamic_cast<Building*>(std::get<1>(zstar[building]))->get_height_base()) {
            building = i;
          }
        }
        //-- store water index
        else if (topoClass == WATER) {
          water = i;
        }
      }

      //-- Deal with buildings. If there's a building and adjacent is not water, then this class
      //-- get allocated the height value of the floor of the building. Any building will do if >1.
      //-- Also ignore water so it doesn't get snapped to the floor of a building
      if (building != -1) {
        //-- push building heights in the node column, both floor and roof heights
        int tmph = -99999;
        for (auto i : buildings) {
          int hfloor = dynamic_cast<Building*>(std::get<1>(zstar[i]))->get_height_base();
          if (std::find(_nc_building_walls[key_bucket].begin(), _nc_building_walls[key_bucket].end(), hfloor) == _nc_building_walls[key_bucket].end()) {
            _nc_building_walls[key_bucket].push_back(hfloor);
          }

          int hroof = dynamic_cast<Building*>(std::get<1>(zstar[i]))->get_height();
          if (std::find(_nc_building_walls[key_bucket].begin(), _nc_building_walls[key_bucket].end(), hroof) == _nc_building_walls[key_bucket].end()) {
            _nc_building_walls[key_bucket].push_back(hroof);
          }
        }
        // add vw to water since it might be lower then the building floor
        if (water != -1) {
          std::get<1>(zstar[water])->add_vertical_wall();
        }
        // get the base height of the building to set as height of the adjacent TopoFeatures
        int baseheight = dynamic_cast<Building*>(std::get<1>(zstar[building]))->get_height_base();
        for (auto& each : zstar) {
          if (std::get<1>(each)->get_class() != BUILDING && std::get<1>(each)->get_class() != WATER) {
            // set buiding base height as height of the adjacent TopoFeature
            std::get<0>(each) = baseheight;
            if (water != -1) {
              //- add a vertical wall between the feature and the water
              std::get<1>(each)->add_vertical_wall();
            }
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
              // this mainly applies for bridges and outlier detection of roads, otherwise it shouldn't be happening
              if (std::get<1>(*it)->get_class() == std::get<1>(*it2)->get_class()) {
                std::get<0>(*it2) = std::get<0>(*it);
              }
              // it is hard, it2 is hard
              // keep height when both are hard surfaces, add vw
              else if (std::get<1>(*it)->is_hard()) {
                if (std::get<1>(*it2)->is_hard()) {
                  //-- add a wall to the heighest feature, it2 is allways highest since zstart is sorted by height
                  std::get<1>(*it2)->add_vertical_wall();
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
            // features are outside threshold jump edges. Fix cases where a feature has no height or add a vertical wall
            else {
              // stitch object withouth height to adjacent object which does have a height
              if (std::get<0>(*it) == -9999 && std::get<0>(*it2) != -9999) {
                std::get<0>(*it) = std::get<0>(*it2);
              }
              else if (std::get<0>(*it2) == -9999 && std::get<0>(*it) != -9999) {
                std::get<0>(*it2) = std::get<0>(*it);
              }
              else {
                //-- add a wall to the heighest feature, it2 is allways highest since zstart is sorted by height
                std::get<1>(*it2)->add_vertical_wall();
              }
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
        int h = std::get<0>(each);
        if (std::get<1>(each)->get_class() != BUILDING) {
          std::get<1>(each)->set_vertex_elevation(std::get<2>(each), std::get<3>(each), h);
        }
        if (h != tmph) { //-- not to repeat the same height
          _nc[key_bucket].push_back(h);
          tmph = h;
        }
      }
    }
  }
}

/**
 * stitch two heights at a vertex with or without jumpedge, depending on object types
 */
void Map3d::stitch_jumpedge(TopoFeature* f1, int ringi1, int pi1, TopoFeature* f2, int ringi2, int pi2) {
  Point2 p = f1->get_point2(ringi1, pi1);
  std::string key_bucket = gen_key_bucket(&p);
  int f1z = f1->get_vertex_elevation(ringi1, pi1);
  int f2z = f2->get_vertex_elevation(ringi2, pi2);

  //-- Buildings involved
  if (f1->get_class() == BUILDING || f2->get_class() == BUILDING) {
    if (f1->get_class() == BUILDING && f2->get_class() == BUILDING) {
      int f1base = dynamic_cast<Building*>(f1)->get_height_base();
      int f2base = dynamic_cast<Building*>(f2)->get_height_base();
      _nc_building_walls[key_bucket].push_back(f1base);
      if (f1base != f2base) {
        _nc_building_walls[key_bucket].push_back(f2base);
      }
      _nc_building_walls[key_bucket].push_back(f1z);
      if (f1z != f2z) {
        _nc_building_walls[key_bucket].push_back(f2z);
      }
    }
    else if (f1->get_class() == BUILDING) {
      int f1base = dynamic_cast<Building*>(f1)->get_height_base();
      if (f2->get_class() != WATER) {
        f2->set_vertex_elevation(ringi2, pi2, f1base);
      }
      else {
        //- keep water flat, add the water height and the building base height to the nc
        _nc[key_bucket].push_back(f2z);
        _nc[key_bucket].push_back(f1base);
      }
      //- expect a building to always be heighest adjacent feature
      _nc_building_walls[key_bucket].push_back(f1base);
      _nc_building_walls[key_bucket].push_back(f1z);
    }
    else { //-- f2 is Building
      int f2base = dynamic_cast<Building*>(f2)->get_height_base();
      if (f1->get_class() != WATER) {
        f1->set_vertex_elevation(ringi1, pi1, f2base);
      }
      else {
        //- keep water flat, add the water height and the building base height to the nc
        _nc[key_bucket].push_back(f1z);
        _nc[key_bucket].push_back(f2base);
      }
      //- expect a building to always be heighest adjacent feature
      _nc_building_walls[key_bucket].push_back(f2base);
      _nc_building_walls[key_bucket].push_back(f2z);
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
        bStitched = true;
      }
      if (f1->is_hard() == false) { //- stitch soft f1 to f2
        f1->set_vertex_elevation(ringi1, pi1, f2z);
        bStitched = true;
      }
      else if (f2->is_hard() == false) { //- stitch soft f2 to f1
        f2->set_vertex_elevation(ringi2, pi2, f1z);
        bStitched = true;
      }
    }
    //-- handle features without height value and vertical walls must be added to highest feature
    if (bStitched == false) {
      // stitch object withouth height to adjacent object which does have a height
      if (f1z == -9999 && f2z != -9999) {
        f1->set_vertex_elevation(ringi1, pi1, f2z);
      }
      else if (f2z == -9999 && f1z != -9999) {
        f2->set_vertex_elevation(ringi2, pi2, f1z);
      }
      else {
        //- add a wall to the heighest feature and push heights to the NodeColumn
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
}

/**
 * stitch two heights at a vertex to their average height
 */
void Map3d::stitch_average(TopoFeature* f1, int ringi1, int pi1, TopoFeature* f2, int ringi2, int pi2) {
  //-- set average height to both features and push height to the NodeColumn
  int avgz = (f1->get_vertex_elevation(ringi1, pi1) + f2->get_vertex_elevation(ringi2, pi2)) / 2;
  f1->set_vertex_elevation(ringi1, pi1, avgz);
  f2->set_vertex_elevation(ringi2, pi2, avgz);
  Point2 p = f1->get_point2(ringi1, pi1);
  _nc[gen_key_bucket(&p)].push_back(avgz);
}

/**
 * main function for stitching features of class Bridge
 * top of a bridge (bridge deck) is forced to be flattened and stitched to the surrounding objects
 * heights of bridge top are collected from height points like other objects
 * bottom of a bridge is stitched to surrounding objects not connected to the top of a bridge
 * heights of bridge bottom are copied from the height of adjacent objects
 */
void Map3d::stitch_bridges() {
  std::vector<int> ringis, pis;
  for (auto& f : _lsFeatures) {
    if (f->get_class() == BRIDGE && f->get_top_level() == false) {
      // Make bridge face flattened
      Bridge* b = dynamic_cast<Bridge*>(f);
      b->detect_outliers(b->get_flatten());

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

        for (int i = 0; i < ring.size(); i++) {
          for (auto& fadj : *lstouching) {
            ringis.clear();
            pis.clear();
            if (!(fadj->get_class() == BRIDGE && fadj->get_top_level()) && fadj->has_point2(ring[i], ringis, pis)) {
              int z = fadj->get_vertex_elevation(ringis[0], pis[0]);
              if (abs(f->get_vertex_elevation(ringi, i) - z) < _threshold_bridge_jump_edges) {
                f->set_vertex_elevation(ringi, i, z);
                if (!(fadj->get_class() == BRIDGE && fadj->get_top_level() == f->get_top_level())) {
                  // Add height to NC
                  Point2 p = f->get_point2(ringi, i);
                  std::string key_bucket = gen_key_bucket(&p);
                  _nc[key_bucket].push_back(z);
                  _bridge_stitches[key_bucket] = z;
                }
              }
            }
          }
        }
      }
    }
  }

  for (auto& f : _lsFeatures) {
    if (f->get_class() == BRIDGE && f->get_top_level()) {
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

        //Search for corners to base stitching on
        //Corners are based on highest level already stitched before
        //and where a bridge is adjacent
        std::vector< std::pair<int, bool> > corners;
        for (int i = 0; i < ring.size(); i++) {
          // find begin of stitched stretch
          Point2 p = f->get_point2(ringi, i);
          std::string key_bucket = gen_key_bucket(&p);

          bool setheight = false;
          int previ = i - 1;
          if (i == 0) {
            previ = ring.size() - 1;
          }
          Point2 prevp = f->get_point2(ringi, previ);
          std::string prev_key_bucket = gen_key_bucket(&prevp);
          if (_bridge_stitches.find(key_bucket) != _bridge_stitches.end() &&
            _bridge_stitches.find(prev_key_bucket) == _bridge_stitches.end()) {
            // add start of stitched stretch to corners
            corners.push_back(std::make_pair(i, true));
            setheight = true;
          }
          else {
            // find end of stitching stretch
            int nexti = i + 1;
            if (i == ring.size() - 1) {
              nexti = 0;
            }
            Point2 nextp = f->get_point2(ringi, nexti);
            std::string next_key_bucket = gen_key_bucket(&nextp);
            if (_bridge_stitches.find(key_bucket) != _bridge_stitches.end() &&
              _bridge_stitches.find(next_key_bucket) == _bridge_stitches.end()) {
              // add end of stitched stretch to corners
              corners.push_back(std::make_pair(i, false));
              setheight = true;
            }
          }
          if (!setheight) { // no corner found yet
            bool bridgeAdj = false;
            bool otherAdj = false;
            //find if there are adjacent bridges
            for (auto& fadj : *lstouching) {
              ringis.clear();
              pis.clear();
              if (fadj->has_point2(ring[i], ringis, pis)) {
                if (fadj->get_class() == BRIDGE && fadj->get_top_level()) {
                  bridgeAdj = true;
                }
                else if (fadj->get_class() != BRIDGE) {
                  otherAdj = true;
                }
              }
            }

            //TODO: Check previous/next vertex like with the _bridge_stitches?
            if (bridgeAdj && otherAdj) {
              // add end of stitched stretch to corners, set heightjump to false to stitch to adjacent object
              corners.push_back(std::make_pair(i, false));
              setheight = true;
            }
          }
          if (setheight) {
            // set corner height to lowest value in the NC
            if (_nc.find(key_bucket) == _nc.end()) {
              std::clog << "ERROR: NodeColumn not filled at " << key_bucket << std::endl;
            }
            int z = _nc[key_bucket].front();
            f->set_vertex_elevation(ringi, i, z);
          }
        }

        // Set height of vertices in between two corners
        for (int c = 0; c < corners.size(); c++) {
          // prepair vertices list to loop for this stretch
          std::pair<int, bool> startCorner = corners[c];
          // set endCorner to first or corners if end of array is reached
          std::pair<int, bool> endCorner = corners.front();
          if (c + 1 < corners.size()) {
            endCorner = corners[c + 1];
          }
          int endCornerIdx = endCorner.first;

          std::vector<int> vertices;
          if (endCornerIdx < startCorner.first) {
            for (int i = startCorner.first + 1; i < ring.size(); i++) {
              vertices.push_back(i);
            }
            for (int i = 0; i < endCornerIdx; i++) {
              vertices.push_back(i);
            }
          }
          else {
            for (int i = startCorner.first + 1; i < endCornerIdx; i++) {
              vertices.push_back(i);
            }
          }

          for (int pi : vertices) {
            Point2 p = f->get_point2(ringi, pi);
            std::string key_bucket = gen_key_bucket(&p);
            int stitchz = 0;
            if (_nc.find(key_bucket) != _nc.end()) {
              stitchz = _nc[key_bucket].front();
            }
            else {
              for (auto& fadj : *lstouching) {
                ringis.clear();
                pis.clear();
                if (fadj->get_class() != BRIDGE && fadj->has_point2(ring[pi], ringis, pis)) {
                  stitchz = fadj->get_vertex_elevation(ringis[0], pis[0]);
                  break;
                }
              }
            }

            if (startCorner.second) { // Stretch where the top is stitched, interpolate between corners and add VW
              // interpolate between start and end corner distance weighted
              int interz = interpolate_height(f, p, ringi, startCorner.first, ringi, endCorner.first);
              // Allways stitch if there is a lower object, otherwise use interpolated height
              if (stitchz < interz) {
                f->set_vertex_elevation(ringi, pi, stitchz);
              }
              else {
                f->set_vertex_elevation(ringi, pi, interz);
                // Add height to NC and add VW
                _nc[key_bucket].push_back(interz);
                f->add_vertical_wall();
              }
            }
            else { // Stretch where the bottom needs to be stitched, find adjacent object within threshold and stitch, otherwise interpolate between previous vertex and next corner
              // check height of previous vertex
              int previ = pi - 1;
              if (pi == 0) {
                previ = ring.size() - 1;
              }

              // interpolate height between previous vertex and next corner distance weighted
              int interz = interpolate_height(f, p, ringi, previ, ringi, endCorner.first);
              int prevz = f->get_vertex_elevation(ringi, previ);
              //Allways stich to lower object or if interpolated between corners within threshold or previous within threshold
              if (stitchz < interz || abs(stitchz - interz) < _threshold_bridge_jump_edges || abs(stitchz - prevz) < _threshold_bridge_jump_edges) {
                f->set_vertex_elevation(ringi, pi, stitchz);
              }
              else {
                f->set_vertex_elevation(ringi, pi, interz);
                // Add height to NC and add VW
                _nc[key_bucket].push_back(interz);
                f->add_vertical_wall();
              }
            }
          }
        }
      }
    }
  }
}

/**
 * interpolate height (distance weighted) between previous and next corner of a bridge
 */
int Map3d::interpolate_height(TopoFeature* f, const Point2 &p, int prevringi, int prevpi, int nextringi, int nextpi) {
  double dprev = distance(p, f->get_point2(prevringi, prevpi));
  double dnext = distance(p, f->get_point2(nextringi, nextpi));
  double dtotal = dprev + dnext;
  return int((dnext / dtotal) * f->get_vertex_elevation(prevringi, prevpi) + (dprev / dtotal) * f->get_vertex_elevation(nextringi, nextpi));
}

void Map3d::add_allowed_las_class(AllowedLASTopo c, int i) {
  _las_classes_allowed[c].insert(i);
}

void Map3d::add_allowed_las_class_within(AllowedLASTopo c, int i) {
  _las_classes_allowed_within[c].insert(i);
}
