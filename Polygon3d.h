
#ifndef __3DFIER__Polygon3D__
#define __3DFIER__Polygon3D__

#include "definitions.h"


class Polygon3d
{
public:
  Polygon3d  (Polygon2d* p, std::string id);
  ~Polygon3d ();
  virtual bool  add_elevation_point(double x, double y, double z) = 0;
  virtual double      get_elevation() = 0;
  std::string   get_id();
  Polygon2d*    get_polygon2d();
  Box           get_bbox2d();

protected:
  Polygon2d*      _p2d;
  std::string     _id;
  int             _no_points;

  // TODO : these should be put in classes that inherits from Polygon3d
  vector<double>   _medianvalues; //-- SIMPLE_MEDIAN method
  vector<Point3d*> _lsPts3d;
};



class Polygon3d_SA : public Polygon3d 
{
public:
  Polygon3d_SA (Polygon2d* p, std::string id);
  bool        add_elevation_point(double x, double y, double z);
  double      get_elevation();
private:
  double      _total_elevation; 
};



#endif /* defined(__boo__Polygon3D__) */
