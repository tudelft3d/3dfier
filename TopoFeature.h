
#ifndef __3DFIER__TopoFeature__
#define __3DFIER__TopoFeature__

#include "definitions.h"
#include "geomtools.h"
#include <random>


class TopoFeature
{
public:
  TopoFeature  (Polygon2* p, std::string pid);
  ~TopoFeature ();

  virtual bool          threeDfy() = 0;
  virtual bool          add_elevation_point(double x, double y, double z, float radius) = 0;
  
  virtual int           get_number_vertices() = 0;
  virtual std::string   get_citygml() = 0;
  virtual std::string   get_obj_v() = 0;
  virtual std::string   get_obj_f(int offset, bool floor = false) = 0;

  std::string  get_id();
  Polygon2*    get_Polygon2();
  Box          get_bbox2d();

protected:
  Polygon2*    _p2;
  std::string  _id;
  std::vector< std::vector<float> > _velevations;
  std::vector<Point3>   _vertices;  //-- output of Triangle
  std::vector<Triangle> _triangles; //-- output of Triangle
  std::vector<Segment>  _segments;  //-- output of Triangle

  bool assign_elevation_to_vertex(double x, double y, double z, float radius);
};


//---------------------------------------------

class Block : public TopoFeature 
{
public:
  Block           (Polygon2* p, std::string pid, std::string heightref_top, std::string heightref_base); 
  bool            threeDfy();
  bool            add_elevation_point(double x, double y, double z, float radius);
  std::string     get_citygml();
  std::string     get_obj_v();
  std::string     get_obj_f(int offset, bool floor = false);
  float           get_height_top();
  float           get_height_base();
  int             get_number_vertices();
  static std::string    _heightref_top;
  static std::string    _heightref_base;
private:
  bool                  _is3d;
  float                 _height_top;
  float                 _height_base;
  std::vector<float>    _zvaluesinside;
  bool build_CDT();
};


//---------------------------------------------

class Boundary3D : public TopoFeature
{
public:
  Boundary3D      (Polygon2* p, std::string pid);
  
  bool            threeDfy();
  bool            add_elevation_point(double x, double y, double z, float radius);
  std::string     get_citygml();
  std::string     get_obj_v();
  std::string     get_obj_f(int offset, bool floor = false);
  int             get_number_vertices();
private:
  int                   _simplification;
  void add_elevations_to_boundary(Polygon3 &p3);
};


//---------------------------------------------

class TIN : public TopoFeature
{
public:
  TIN             (Polygon2* p, std::string pid, int simplification = 0);
  bool            threeDfy();
  bool            add_elevation_point(double x, double y, double z, float radius);
  std::string     get_citygml();
  std::string     get_obj_v();
  std::string     get_obj_f(int offset, bool floor = false);
  int             get_number_vertices();
private:
  int                   _simplification;
  std::vector<Point3>   _lidarpts;
  void add_elevations_to_boundary(Polygon3 &p3);
};


#endif 
