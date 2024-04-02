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

#ifndef __3dfier__Map3d__
#define __3dfier__Map3d__

#include "definitions.h"
#include "geomtools.h"
#include "io.h"
#include "TopoFeature.h"
#include "Building.h"
#include "Terrain.h"
#include "Forest.h"
#include "Water.h"
#include "Road.h"
#include "Separation.h"
#include "Bridge.h"
#include "boost/locale.hpp"

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
  void add_elevation_point(LASpoint const& laspt);
  void cleanup_elevations();

  unsigned long get_num_polygons();
  const std::vector<TopoFeature*>&  get_polygons3d();
  Box2 get_bbox();
  bool check_bounds(const double xmin, const double xmax, const double ymin, const double ymax);

  void get_citygml(std::wostream& of);
  void get_citygml_multifile(std::string);
  void create_citygml_header(std::wostream& of);
  void get_citygml_imgeo(std::wostream& of);
  bool get_cityjson(std::wostream& of);
  void get_citygml_imgeo_multifile(std::string ofname);
  void create_citygml_imgeo_header(std::wostream& of);
  bool get_postgis_output(std::string filename, bool pdok = false, bool citygml = false);
  bool get_gdal_output(std::string filename, std::string drivername, bool multi);
  void get_csv_buildings(std::wostream& of);
  void get_csv_buildings_multiple_heights(std::wostream& of);
  void get_csv_buildings_all_elevation_points(std::wostream& of);
  void get_obj_per_feature(std::wostream& of);
  void get_obj_per_class(std::wostream& of);
  void get_stl(std::wostream& of);

  void set_building_heightref_roof(float heightref);
  void set_building_heightref_ground(float heightref);
  void set_building_include_floor(bool include);
  void set_building_triangulate(bool triangulate);
  void set_building_inner_walls(bool inner_walls);
  void set_building_lod(int lod);
  void set_terrain_simplification(int simplification);
  void set_forest_simplification(int simplification);
  void set_terrain_simplification_tinsimp(double tinsimp_threshold);
  void set_forest_simplification_tinsimp(double tinsimp_threshold);
  void set_terrain_innerbuffer(float innerbuffer);
  void set_forest_innerbuffer(float innerbuffer);
  void set_water_heightref(float heightref);
  void set_road_heightref(float heightref);
  void set_road_filter_outliers(bool filter);
  void set_road_flatten(bool flatten);
  void set_separation_heightref(float heightref);
  void set_bridge_heightref(float heightref);
  void set_bridge_flatten(bool flatten);
  void set_radius_vertex_elevation(float radius);
  void set_building_radius_vertex_elevation(float radius);
  void set_threshold_jump_edges(float threshold);
  void set_threshold_bridge_jump_edges(float threshold);
  void set_requested_extent(double xmin, double ymin, double xmax, double ymax);
  void set_max_angle_curvepolygon(double max_angle);

  void add_allowed_las_class(AllowedLASTopo c, int i);
  void add_allowed_las_class_within(AllowedLASTopo c, int i);
  bool save_building_variables();
  int interpolate_height(TopoFeature* f, const Point2 &p, int prevringi, int prevpi, int nextringi, int nextpi);

private:
  float       _building_heightref_roof;
  float       _building_heightref_ground;
  bool        _building_triangulate;
  int         _building_lod;
  bool        _building_include_floor;
  bool        _building_inner_walls;
  int         _terrain_simplification;
  int         _forest_simplification;
  double      _terrain_simplification_tinsimp;
  double      _forest_simplification_tinsimp;
  float       _terrain_innerbuffer;
  float       _forest_innerbuffer;
  float       _water_heightref;
  float       _road_heightref;
  bool        _road_filter_outliers;
  bool        _road_flatten;
  float       _separation_heightref;
  float       _bridge_heightref;
  bool        _bridge_flatten;
  float       _radius_vertex_elevation;
  float       _building_radius_vertex_elevation;
  int         _threshold_jump_edges; //-- in cm/integer
  int         _threshold_bridge_jump_edges; //-- in cm/integer
  Box2        _bbox;
  double      _minxradius;
  double      _maxxradius;
  double      _minyradius;
  double      _maxyradius;
  Box2        _requestedExtent;
  double      _max_angle_curvepolygon; //-- the largest step in degrees along the arc, zero to use the default setting.

  //-- storing the LAS allowed for each TopoFeature
  std::array<std::set<int>,NUM_ALLOWEDLASTOPO> _las_classes_allowed;
  std::array<std::set<int>,NUM_ALLOWEDLASTOPO> _las_classes_allowed_within;

  NodeColumn                                          _nc;
  NodeColumn                                          _nc_building_walls;
  std::unordered_map<std::string, int>                _bridge_stitches;
  std::vector<TopoFeature*>                           _lsFeatures;
  bgi::rtree< PairIndexed, bgi::rstar<16> >           _rtree;
  bgi::rtree< PairIndexed, bgi::rstar<16> >           _rtree_buildings;

#if GDAL_VERSION_MAJOR < 2
  bool extract_and_add_polygon(OGRDataSource* dataSource, PolygonFile* file);
#else
  bool extract_and_add_polygon(GDALDataset* dataSource, PolygonFile* file);
  OGRLayer* create_gdal_layer(GDALDriver* driver, GDALDataset* dataSource, std::string filename, std::string layername, AttributeMap attributes, bool addHeightAttributes);
#endif
  void extract_feature(OGRFeature * f, std::string layerName, const char * idfield, const char * heightfield, std::string layertype, bool multiple_heights);
  void stitch_one_vertex(TopoFeature* f, int ringi, int pi, std::vector< std::tuple<TopoFeature*, int, int> >& star);
  void stitch_jumpedge(TopoFeature* f1, int ringi1, int pi1, TopoFeature* f2, int ringi2, int pi2);
  void stitch_average(TopoFeature* f1, int ringi1, int pi1, TopoFeature* f2, int ringi2, int pi2);
  void stitch_bridges();
  void collect_adjacent_features(TopoFeature* f);
};

#endif
