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

#ifndef __3dfier__Map3d__
#define __3dfier__Map3d__

#include "definitions.h"
#include "geomtools.h"
#include "TopoFeature.h"
#include "Building.h"
#include "Terrain.h"
#include "Forest.h"
#include "Water.h"
#include "Road.h"
#include "Separation.h"
#include "Bridge.h"

#include <set>
#include <array>

typedef std::pair<Box2, TopoFeature*> PairIndexed;

class Map3d {
public:
  Map3d();
  ~Map3d();

  bool add_polygons_files(std::vector<PolygonFile> &files);
  bool add_las_file(PointFile pointFile);

  void stitch_lifted_features();
  bool construct_rtree();
  bool threeDfy(bool stitching = true);
  bool construct_CDT();
  void add_elevation_point(liblas::Point const& laspt);

  unsigned long get_num_polygons();
  const std::vector<TopoFeature*>&  get_polygons3d();
  Box2 get_bbox();
  liblas::Bounds<double> get_bounds();

  void get_citygml(std::wostream& of);
  void get_citygml_multifile(std::string);
  void create_citygml_header(std::wostream& of);
  void get_citygml_imgeo(std::wostream& of);
  bool get_cityjson(std::string filename);
  void get_citygml_imgeo_multifile(std::string ofname);
  void create_citygml_imgeo_header(std::wostream& of);
  bool get_pdok_output(std::string filename);
  bool get_gdal_output(std::string filename, std::string drivername, bool multi);
  void get_csv_buildings(std::wostream& of);
  void get_csv_buildings_multiple_heights(std::wostream& of);
  void get_csv_buildings_all_elevation_points(std::wostream& of);
  void get_obj_per_feature(std::wostream& of);
  void get_obj_per_class(std::wostream& of);
  bool get_shapefile2d(std::string filename);

  void set_building_heightref_roof(float heightref);
  void set_building_heightref_ground(float heightref);
  void set_building_include_floor(bool include);
  void set_building_triangulate(bool triangulate);
  void set_building_lod(int lod);
  void set_terrain_simplification(int simplification);
  void set_forest_simplification(int simplification);
  void set_terrain_simplification_tinsimp(double tinsimp_threshold);
  void set_forest_simplification_tinsimp(double tinsimp_threshold);
  void set_terrain_innerbuffer(float innerbuffer);
  void set_forest_innerbuffer(float innerbuffer);
  void set_water_heightref(float heightref);
  void set_road_heightref(float heightref);
  void set_road_threshold_outliers(int t);
  void set_separation_heightref(float heightref);
  void set_bridge_heightref(float heightref);
  void set_radius_vertex_elevation(float radius);
  void set_building_radius_vertex_elevation(float radius);
  void set_threshold_jump_edges(float threshold);
  void set_requested_extent(double xmin, double ymin, double xmax, double ymax);

  void add_allowed_las_class(AllowedLASTopo c, int i);
  bool save_building_variables();
  int interpolate_height(TopoFeature* f, const Point2 &p, int prevringi, int prevpi, int nextringi, int nextpi);

private:
  float       _building_heightref_roof;
  float       _building_heightref_ground;
  bool        _building_triangulate; // TODO: Not used anymore, remove? 
  int         _building_lod;
  bool        _building_include_floor; // TODO: Not used anymore, remove? 
  int         _terrain_simplification;
  int         _forest_simplification;
  double      _terrain_simplification_tinsimp;
  double      _forest_simplification_tinsimp;
  float       _terrain_innerbuffer;
  float       _forest_innerbuffer;
  float       _water_heightref;
  float       _road_heightref;
  int         _road_threshold_outliers;
  float       _separation_heightref;
  float       _bridge_heightref;
  float       _radius_vertex_elevation;
  float       _building_radius_vertex_elevation;
  int         _threshold_jump_edges; //-- in cm/integer
  Box2        _bbox;
  Box2        _requestedExtent;

  //-- storing the LAS allowed for each TopoFeature
  std::array<std::set<int>,NUM_ALLOWEDLASTOPO> _las_classes_allowed;

  NodeColumn                                          _nc;
  std::vector<TopoFeature*>                           _lsFeatures;
  std::vector<std::string>                            _allowed_layers;
  bgi::rtree< PairIndexed, bgi::rstar<16> >           _rtree;
  bgi::rtree< PairIndexed, bgi::rstar<16> >           _rtree_buildings;

#if GDAL_VERSION_MAJOR < 2
  bool extract_and_add_polygon(OGRDataSource* dataSource, PolygonFile* file);
#else
  bool extract_and_add_polygon(GDALDataset* dataSource, PolygonFile* file);
  OGRLayer* create_gdal_layer(GDALDriver *driver, std::string filename, std::string layername, AttributeMap attributes, bool addHeightAttributes);
  void close_gdal_resources(GDALDriver* driver, std::unordered_map<std::string, OGRLayer*> layers);
#endif
  void extract_feature(OGRFeature * f, std::string layerName, const char * idfield, const char * heightfield, std::string layertype, bool multiple_heights);
  void stitch_one_vertex(TopoFeature* f, int ringi, int pi, std::vector< std::tuple<TopoFeature*, int, int> >& star);
  void stitch_jumpedge(TopoFeature* f1, int ringi1, int pi1, TopoFeature* f2, int ringi2, int pi2);
  void stitch_average(TopoFeature* f1, int ringi1, int pi1, TopoFeature* f2, int ringi2, int pi2);
  void stitch_bridges();
  void collect_adjacent_features(TopoFeature* f);
};

#endif
