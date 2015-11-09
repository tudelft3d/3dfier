
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

  virtual bool          lift() = 0;
  virtual bool          buildCDT() = 0;
  virtual bool          add_elevation_point(double x, double y, double z, float radius) = 0;
  virtual std::string   get_citygml() = 0;
  virtual std::string   get_obj_v();
  virtual std::string   get_obj_f(int offset);
  virtual int           get_number_vertices() = 0;
  virtual TopoClass     get_class() = 0;

  std::string  get_id();
  int          get_counter(); 
  Polygon2*    get_Polygon2();
  Box2         get_bbox2d();
  bool         has_segment(Point2& a, Point2& b, int& ia, int& ib);
  void         set_point_elevation(int i, float z);  
  float        get_point_elevation(int i);
  int          has_point2(Point2& p);

protected:
  Polygon2*    _p2;
  Polygon3     _p3;
  std::string  _id;
  int          _counter;
  static int   _count;
  std::vector< std::vector<float> > _velevations;
  std::vector<Point3>   _vertices;  //-- output of Triangle
  std::vector<Triangle> _triangles; //-- output of Triangle
  std::vector<Segment>  _segments;  //-- output of Triangle

  Point2& get_previous_point2(int i, int& index);
  bool    assign_elevation_to_vertex(double x, double y, double z, float radius);
  void    lift_vertices_boundary(float percentile);
};


//---------------------------------------------

class Block : public TopoFeature 
{
public:
  Block           (Polygon2* p, std::string pid, std::string heightref_top, std::string heightref_base); 
  virtual bool    lift() = 0;
  bool            add_elevation_point(double x, double y, double z, float radius);
  virtual std::string   get_citygml() = 0;
  std::string     get_obj_v();
  std::string     get_obj_f(int offset);
  int             get_number_vertices();
  virtual TopoClass     get_class() = 0;
  
  float           get_height_top();
  float           get_height_base();
  static std::string    _heightref_top;
  static std::string    _heightref_base;
protected:
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
  Boundary3D    (Polygon2* p, std::string pid);
  virtual bool  lift() = 0;
  virtual bool  buildCDT() = 0;
  virtual std::string   get_citygml() = 0;
  bool          add_elevation_point(double x, double y, double z, float radius);
  int           get_number_vertices();
  virtual TopoClass     get_class() = 0;
protected:
  int           _simplification;
  void          smooth_boundary(int passes = 1);
  void          smooth_ring(Ring3 &r, std::vector<float> &elevs);
};


//---------------------------------------------

class TIN : public TopoFeature
{
public:
  TIN             (Polygon2* p, std::string pid, int simplification = 0);
  virtual bool    lift() = 0;
  virtual bool    buildCDT() = 0;
  virtual std::string   get_citygml() = 0;
  bool            add_elevation_point(double x, double y, double z, float radius);
  int             get_number_vertices();
  virtual TopoClass     get_class() = 0;
protected:
  int                   _simplification;
  std::vector<Point3>   _lidarpts;
};


#endif 
