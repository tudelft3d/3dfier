
#ifndef __3dfier__Map3d__
#define __3dfier__Map3d__

#include "definitions.h"
#include "Polygon3d.h"

typedef std::pair<Box, Polygon3d*> PairIndexed;

class Map3d
{
public:
  Map3d  ();
  ~Map3d ();


  bool add_polygons_file(std::string ifile, std::string idfield, std::vector<std::pair<std::string, std::string> > &layers);
  bool add_polygons_file(std::string ifile, std::string idfield, std::string lifttype);

  bool add_las_file(std::string ifile, int skip = 0);
//  bool add_point(Point2d* q);

  bool construct_rtree();
  bool threeDfy();
  Polygon3d* add_point(double x, double y, double z, Polygon3d* trythisone = NULL);

  unsigned long get_num_polygons();
  const std::vector<Polygon3d*>& get_polygons3d();  
  
  std::string get_citygml();
  std::string get_csv();
  std::string get_obj_v();
  std::string get_obj_f();

     
private:
  std::vector<Polygon3d*>   _lsPolys;
  std::vector<std::string>  _allowed_layers;
  bgi::rtree< PairIndexed, bgi::rstar<16> > _rtree;

  bool extract_and_add_polygon(OGRDataSource *dataSource, std::string idfield, std::vector< std::pair<std::string, std::string> > &layers);

};


#endif