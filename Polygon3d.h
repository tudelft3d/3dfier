
#ifndef __3DFIER__Polygon3D__
#define __3DFIER__Polygon3D__

#include "definitions.h"
#include "geomtools.h"


class Polygon3d
{
public:
  Polygon3d  (Polygon2d* p, std::string id);
  ~Polygon3d ();

  virtual bool          add_elevation_point(double x, double y, double z) = 0;
  virtual std::string   get_lift_type() = 0;
  virtual std::string   get_3d_citygml() = 0;
  virtual std::string   get_3d_csv() = 0;

  std::string   get_id();
  Polygon2d*    get_polygon2d();
  Box           get_bbox2d();

protected:
  Polygon2d*      _p2;
  std::string     _id;
  
 // vector<Point3d*> _lsPts3d;
};

//---------------------------------------------

class Polygon3dBlock : public Polygon3d 
{
public:
  Polygon3dBlock (Polygon2d* p, std::string id, std::string lifttype); 
  
  bool            add_elevation_point(double x, double y, double z);
  std::string     get_3d_citygml();
  std::string     get_3d_csv();
  double          get_height();
  std::string     get_lift_type();
private:
  std::vector<double> _zvalues;
  std::string         _lifttype;
};


//---------------------------------------------

class Polygon3dBoundary : public Polygon3d
{
public:
  Polygon3dBoundary (Polygon2d* p, std::string id);
  
  bool            add_elevation_point(double x, double y, double z);
  std::string     get_3d_citygml();
  std::string     get_3d_csv();
  std::string     get_lift_type();
private:
  std::vector<Point3d>  _lidarpts;
  std::vector<Point3d>  _vertices;
  std::vector<Triangle> _triangles;

  bool build_CDT();
};


//---------------------------------------------

class Polygon3dTin : public Polygon3d
{
public:
  Polygon3dTin (Polygon2d* p, std::string id, std::string lifttype);
  
  bool            add_elevation_point(double x, double y, double z);
  std::string     get_3d_citygml();
  std::string     get_3d_csv();
  std::string     get_lift_type();
private:
  std::string           _lifttype;
  std::vector<Point3d>  _lidarpts;
  std::vector<Point3d>  _vertices;
  std::vector<Triangle> _triangles;
  bool build_CDT();
};


#endif /* defined(__boo__Polygon3D__) */
