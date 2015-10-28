
#ifndef __3dfier__Map3d__
#define __3dfier__Map3d__

#include "definitions.h"
#include "TopoFeature.h"
#include "Building.h"
#include "Terrain.h"
#include "Vegetation.h"
#include "Water.h"
#include "Road.h"

typedef std::pair<Box2, TopoFeature*> PairIndexed;

class Map3d
{
public:
  Map3d  ();
  ~Map3d ();

  bool add_polygons_file(std::string ifile, std::string idfield, std::vector<std::pair<std::string, std::string> > &layers);
  bool add_polygons_file(std::string ifile, std::string idfield, std::string lifttype);

  bool add_las_file(std::string ifile, int skip = 0);

  void stitch_lifted_features();
  bool construct_rtree();
  bool threeDfy();
  TopoFeature* add_elevation_point(double x, double y, double z, TopoFeature* trythisone = NULL);


  unsigned long get_num_polygons();
  const std::vector<TopoFeature*>& get_polygons3d();  
  
  std::string get_citygml();
  std::string get_csv_buildings();
  std::string get_obj();
     
  void set_building_heightref_roof(std::string heightref);
  void set_building_heightref_floor(std::string heightref);
  void set_building_include_floor(bool include);
  void set_building_triangulate(bool triangulate);
  void set_terrain_simplification(int simplification);
  void set_vegetation_simplification(int simplification);
  void set_water_heightref(std::string heightref);
  void set_road_heightref(std::string heightref);
  void set_radius_vertex_elevation(float radius);
private:
  std::string _building_heightref_roof;
  std::string _building_heightref_floor;
  bool        _building_triangulate;
  bool        _building_include_floor;
  int         _terrain_simplification;
  int         _vegetation_simplification;
  std::string _water_heightref;
  std::string _road_heightref;
  float       _radius_vertex_elevation;

  std::vector<TopoFeature*>   _lsPolys;
  std::vector<std::string>    _allowed_layers;
  bgi::rtree< PairIndexed, bgi::rstar<16> > _rtree;

  bool extract_and_add_polygon(OGRDataSource *dataSource, std::string idfield, std::vector< std::pair<std::string, std::string> > &layers);

};


#endif