
#ifndef __3DFIER__Polygon3D__
#define __3DFIER__Polygon3D__

#include "definitions.h"
#include "geomtools.h"
#include <random>


class Polygon3d
{
public:
  Polygon3d  (Polygon2* p, std::string pid);
  ~Polygon3d ();

  virtual bool          threeDfy() = 0;
  virtual bool          add_elevation_point(double x, double y, double z) = 0;
  virtual std::string   get_lift_type() = 0;
  virtual std::string   get_3d_citygml() = 0;
  virtual std::string   get_3d_csv() = 0;
  virtual std::string   get_obj_v() = 0;
  virtual std::string   get_obj_f(int offset, bool floor = false) = 0;
  virtual int           get_number_vertices() = 0;

  std::string  get_id();
  Polygon2*    get_Polygon2();
  Box          get_bbox2d();

protected:
  Polygon2*    _p2;
  std::string  _id;
};

//---------------------------------------------

class Polygon3dBlock : public Polygon3d 
{
public:
  Polygon3dBlock (Polygon2* p, std::string pid, std::string lifttype); 
  
  bool            threeDfy();
  bool            add_elevation_point(double x, double y, double z);
  std::string     get_3d_citygml();
  std::string     get_3d_csv();
  std::string     get_obj_v();
  std::string     get_obj_f(int offset, bool floor = false);
  float           get_roof_height();
  float           get_floor_height();
  std::string     get_lift_type();
  int             get_number_vertices();
private:
  bool                  _is3d;
  float                 _floorheight;
  float                 _roofheight;
  std::vector<float>    _zvaluesinside;
  std::vector<float>    _vertexelevations;
  std::string           _lifttype;
  std::vector<Point3>   _vertices;
  std::vector<Triangle> _triangles;
  std::vector<Segment>  _segments;
  bool assign_elevation_to_vertex(double x, double y, double z);
  bool build_CDT();
};


//---------------------------------------------

class Polygon3dBoundary : public Polygon3d
{
public:
  Polygon3dBoundary (Polygon2* p, std::string pid);
  
  bool            threeDfy();
  bool            add_elevation_point(double x, double y, double z);
  std::string     get_3d_citygml();
  std::string     get_3d_csv();
  std::string     get_obj_v();
  std::string     get_obj_f(int offset, bool floor = false);
  std::string     get_lift_type();
  int             get_number_vertices();
private:
  std::vector<Point3>   _lidarpts;
  std::vector<Point3>   _vertices;
  std::vector<Triangle> _triangles;
  std::vector<Segment>  _segments;
};


//---------------------------------------------

class Polygon3dTin : public Polygon3d
{
public:
  Polygon3dTin (Polygon2* p, std::string pid, std::string lifttype);
  
  bool            threeDfy();
  bool            add_elevation_point(double x, double y, double z);
  std::string     get_3d_citygml();
  std::string     get_3d_csv();
  std::string     get_obj_v();
  std::string     get_obj_f(int offset, bool floor = false);
  std::string     get_lift_type();
  int             get_number_vertices();
private:
  std::string           _lifttype;
  int                   _thin_factor;
  std::vector<Point3>   _lidarpts;
  std::vector<Point3>   _vertices;  //-- output of Triangle
  std::vector<Triangle> _triangles; //-- output of Triangle
  std::vector<Segment>  _segments;  //-- output of Triangle

  std::vector< std::tuple<int, float> > _vertexelevations;
  bool assign_elevation_to_vertex(double x, double y, double z);
  void add_elevations_to_boundary(Polygon3 &p3);
};


#endif /* defined(__boo__Polygon3D__) */
