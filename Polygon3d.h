
#ifndef __3DFIER__Polygon3D__
#define __3DFIER__Polygon3D__

#include "definitions.h"


class Polygon3d
{
public:
  Polygon3d  ();
  Polygon3d  (Polygon2d* p, ExtrusionType type, std::string id);
  ~Polygon3d ();

  bool        add_elevation_point(double x, double y, double z);

  double      get_elevation();

  Box         get_bbox2d();
  std::string get_id();
  Polygon2d*  get_polygon2d();
     
private:
  Polygon2d*      _p2d;
  std::string     _id;
  ExtrusionType   _extrusiontype;
  int             _no_points;

  // TODO : these should be put in classes that inherits from Polygon3d
  double           _total_elevation; //-- SIMPLE_AVG method
  vector<double>   _medianvalues; //-- SIMPLE_MEDIAN method
  vector<Point3d*> _lsPts3d;
};


#endif /* defined(__boo__Polygon3D__) */
