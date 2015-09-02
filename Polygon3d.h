
#ifndef __3DFIER__Polygon3D__
#define __3DFIER__Polygon3D__

#include "definitions.h"


class Polygon3d
{
public:
  Polygon3d  (Polygon2d* p, std::string id);
  ~Polygon3d ();

  virtual ExtrusionType get_extrusion_type() = 0;
  virtual bool          add_elevation_point(double x, double y, double z) = 0;
  virtual std::string   get_citygml() = 0;
  virtual std::string   get_3d_representation() = 0;

  std::string   get_id();
  Polygon2d*    get_polygon2d();
  Box           get_bbox2d();

protected:
  Polygon2d*      _p2d;
  std::string     _id;
  int             _no_points;
 // vector<Point3d*> _lsPts3d;
};


class Polygon3d_H_AVG : public Polygon3d 
{
public:
  Polygon3d_H_AVG (Polygon2d* p, std::string id);
  bool          add_elevation_point(double x, double y, double z);
  ExtrusionType get_extrusion_type();
  std::string   get_citygml();
  std::string   get_3d_representation();
  double        get_height();
private:
  double      _total_elevation; 
};


class Polygon3d_H_MEDIAN : public Polygon3d 
{
public:
  Polygon3d_H_MEDIAN (Polygon2d* p, std::string id);
  bool          add_elevation_point(double x, double y, double z);
  ExtrusionType get_extrusion_type();
  std::string   get_citygml();
  std::string   get_3d_representation();
  double        get_height();
private:
  std::vector<double> _medianvalues; 
};



#endif /* defined(__boo__Polygon3D__) */
