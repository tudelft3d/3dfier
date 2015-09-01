
#ifndef __3dfier__Map3d__
#define __3dfier__Map3d__

#include "definitions.h"
#include "Polygon3d.h"

typedef std::pair<Box, Polygon3d*> PairIndexed;

class Map3d
{
public:
  Map3d  ();
  Map3d  (std::vector<std::string> allowed_layers);
  ~Map3d ();

  bool add_allowed_layer(std::string l);
  bool add_allowed_layers(std::vector<std::string> ls);
  
  bool add_gml_file(std::string ifile, std::string idfield);
  bool add_shp_file(std::string ifile, std::string idfield);
  bool add_las_file(std::string ifile);

  bool construct_rtree();
  bool add_polygon3d(Polygon3d* pgn);
//  bool add_point(Point2d* q);
  Polygon3d* add_point(double x, double y, double z, Polygon3d* trythisone = NULL);

  unsigned long get_num_polygons();

  const std::vector<Polygon3d*>& get_polygons3d();  
     
private:
  std::vector<Polygon3d*>             _lsPolys;
  bgi::rtree< PairIndexed, bgi::rstar<16> > _rtree;
  std::vector<std::string>            _allowed_layers;
};


#endif