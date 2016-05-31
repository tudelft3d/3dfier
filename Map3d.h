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

typedef std::pair<Box2, TopoFeature*> PairIndexed;

class Map3d
{
public:
  Map3d  ();
  ~Map3d ();

  bool add_polygons_file(std::string ifile, std::string idfield, std::string heightfield, std::vector< std::pair<std::string, std::string> > &layers);
  bool add_polygons_file(std::string ifile, std::string idfield, std::string heightfield, std::string lifting);

  bool add_las_file(std::string ifile, std::vector<int> lasomits, int skip = 0);

  void stitch_lifted_features();
  bool construct_rtree();
  bool threeDfy(bool triangulate = true);
  void add_elevation_point(liblas::Point const& laspt);
  // void add_elevation_point(double x, double y, double z, int returnno, liblas::Classification lasclass);

  unsigned long get_num_polygons();
  const std::vector<TopoFeature*>& get_polygons3d();  
  
  std::string get_citygml();
  std::string get_csv_buildings();
  std::string get_obj_per_feature(int z_exaggeration = 0);
  std::string get_obj_per_class(int z_exaggeration = 0);
     
  void set_building_heightref_roof(std::string heightref);
  void set_building_heightref_floor(std::string heightref);
  void set_building_include_floor(bool include);
  void set_building_triangulate(bool triangulate);
  void set_terrain_simplification(int simplification);
  void set_forest_simplification(int simplification);
  void set_water_heightref(std::string heightref);
  void set_road_heightref(std::string heightref);
  void set_radius_vertex_elevation(float radius);
  void set_threshold_jump_edges(float threshold);
private:
  std::string _building_heightref_roof;
  std::string _building_heightref_floor;
  bool        _building_triangulate;
  bool        _building_include_floor;
  int         _terrain_simplification;
  int         _forest_simplification;
  std::string _water_heightref;
  std::string _road_heightref;
  float       _radius_vertex_elevation;
  int         _threshold_jump_edges; //-- in cm/integer
  double      _minx;
  double      _miny;

  std::unordered_multimap<std::string, int> _nc;
  std::vector<TopoFeature*>                 _lsFeatures;
  std::vector<std::string>                  _allowed_layers;
  bgi::rtree< PairIndexed, bgi::rstar<16> > _rtree;

#if GDAL_VERSION_MAJOR < 2
  bool extract_and_add_polygon(OGRDataSource *dataSource, std::string idfield, std::string heightfield, std::vector< std::pair<std::string, std::string> > &layers);
#else
  bool extract_and_add_polygon(GDALDataset *dataSource, std::string idfield, std::string heightfield, std::vector< std::pair<std::string, std::string> > &layers);
#endif

  void stitch_one_feature(TopoFeature* f, TopoClass adjclass);
  bool read_one_gdal_layer(OGRLayer* dataLayer, std::string idfield, std::pair<std::string, std::string> &l);
  void stitch_one_vertex(TopoFeature* f, int ringi, int pi, std::vector< std::tuple<TopoFeature*, int, int> >& star);
  void stitch_jumpedge(TopoFeature* f1, int ringi1, int pi1, TopoFeature* f2, int ringi2, int pi2);
  void stitch_average(TopoFeature* f1, int ringi1, int pi1, TopoFeature* f2, int ringi2, int pi2);
  std::vector<TopoFeature*> get_adjacent_features(TopoFeature* f);

};

#endif