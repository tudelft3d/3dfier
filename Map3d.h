
#ifndef __3dfier__Map3d__
#define __3dfier__Map3d__

#include "definitions.h"
#include "TopoFeature.h"
#include "Building.h"

typedef std::pair<Box, TopoFeature*> PairIndexed;

class Map3d
{
public:
  Map3d  ();
  ~Map3d ();

  bool add_polygons_file(std::string ifile, std::string idfield, std::vector<std::pair<std::string, std::string> > &layers);
  bool add_polygons_file(std::string ifile, std::string idfield, std::string lifttype);

  bool add_las_file(std::string ifile, int skip = 0);
//  bool add_point(Point2* q);

  bool construct_rtree();
  bool threeDfy();
  TopoFeature* add_elevation_point(double x, double y, double z, double buffer = 2.0, TopoFeature* trythisone = NULL);

  unsigned long get_num_polygons();
  const std::vector<TopoFeature*>& get_polygons3d();  
  
  std::string get_citygml();
  std::string get_csv();
  std::string get_obj();
     
  void set_building_height_reference(std::string heightref);
  void set_building_include_floor(bool include);
  void set_building_triangulate(bool triangulate);
  void set_terrain_simplification(int simplification);
  void set_vegetation_simplification(int simplification);
private:
  std::string _building_heightref;
  bool        _building_triangulate;
  bool        _building_include_floor;
  int         _terrain_simplification;
  int         _vegetation_simplification;

  std::vector<TopoFeature*>   _lsPolys;
  std::vector<std::string>    _allowed_layers;
  bgi::rtree< PairIndexed, bgi::rstar<16> > _rtree;

  bool extract_and_add_polygon(OGRDataSource *dataSource, std::string idfield, std::vector< std::pair<std::string, std::string> > &layers);

};


#endif