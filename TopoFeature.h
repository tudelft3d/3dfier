
/*
 Copyright (c) 2015 Hugo Ledoux
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#ifndef __3DFIER__TopoFeature__
#define __3DFIER__TopoFeature__

#include "definitions.h"
#include "geomtools.h"
#include <random>



class TopoFeature
{
public:
  TopoFeature  (char *wkt, std::string pid);
  ~TopoFeature ();

  virtual bool          lift() = 0;
  virtual bool          buildCDT() = 0;
  virtual bool          add_elevation_point(double x, double y, double z, float radius, bool lastreturn = true) = 0;
  virtual std::string   get_citygml() = 0;
  virtual std::string   get_obj_v(int z_exaggeration);
  virtual std::string   get_obj_f(int offset, bool usemtl);
  virtual int           get_number_vertices() = 0;
  virtual TopoClass     get_class() = 0;
  virtual bool          is_hard() = 0;

  std::string  get_id();
  void         construct_vertical_walls(std::vector<TopoFeature*> lsAdj);
  void         fix_bowtie(std::vector<TopoFeature*> lsAdj);
  int          get_counter(); 
  Polygon2*    get_Polygon2();
  Box2         get_bbox2d();
  bool         has_point2(Point2& p, int& ringi, int& pi);
  int          get_point_elevation(int ringi, int pi);
  void         set_point_elevation(int ringi, int pi, int z);  
  bool         has_segment(Point2& a, Point2& b);
  void         set_top_level(bool toplevel);
  void         add_nc(int i, float z);
  bool         has_vertical_walls(); 
  bool         get_top_level();
  std::vector<float>& get_nc(int i);

protected:
  Polygon2*    _p2;
  Polygon3     _p3;
  std::string  _id;
  int          _counter;
  static int   _count;
  bool         _toplevel; 

  std::vector< std::vector<float> > _nc; //-- node columns
  std::vector< std::vector<float> > _velevations;

  std::vector< std::vector< std::vector<int> > > _lidarelevs;
  std::vector< std::vector<int> >                _z;

  std::vector<Point3>   _vertices;  //-- output of Triangle
  std::vector<Triangle> _triangles; //-- output of Triangle
  std::vector<Point3>   _vertices_vw;  //-- for vertical walls
  std::vector<Triangle> _triangles_vw; //-- for vertical walls

  Point2  get_next_point2_in_ring(int ringi, int& pi);
  bool    assign_elevation_to_vertex(double x, double y, double z, float radius);
  void    lift_vertices_boundary(float percentile);
};


//---------------------------------------------

class Block : public TopoFeature 
{
public:
  Block           (char *wkt, std::string pid, std::string heightref_top, std::string heightref_base); 
  virtual bool        lift() = 0;
  bool                add_elevation_point(double x, double y, double z, float radius, bool lastreturn);
  virtual std::string get_citygml() = 0;
  std::string         get_obj_v(int z_exaggeration);
  std::string         get_obj_f(int offset, bool usemtl);
  int                 get_number_vertices();
  virtual TopoClass   get_class() = 0;
  virtual bool        is_hard() = 0;
  
  int               get_height_top();
  int               get_height_base();
  static std::string  _heightref_top;
  static std::string  _heightref_base;
protected:
  bool                _is3d;
  float               _height_top;
  float               _height_base;
  std::vector<int>    _zvaluesinside;
  bool build_CDT();
};


//---------------------------------------------

class Boundary3D : public TopoFeature
{
public:
  Boundary3D    (char *wkt, std::string pid);
  virtual bool  lift() = 0;
  virtual bool  buildCDT() = 0;
  virtual std::string   get_citygml() = 0;
  bool          add_elevation_point(double x, double y, double z, float radius, bool lastreturn = true);
  int           get_number_vertices();
  virtual TopoClass     get_class() = 0;
  virtual bool          is_hard() = 0;
protected:
  int           _simplification;
  void          smooth_boundary(int passes = 1);
  void          smooth_ring(Ring3 &r, std::vector<float> &elevs);
};


//---------------------------------------------

class TIN : public TopoFeature
{
public:
  TIN             (char *wkt, std::string pid, int simplification = 0);
  virtual bool    lift() = 0;
  virtual bool    buildCDT() = 0;
  virtual std::string   get_citygml() = 0;
  bool            add_elevation_point(double x, double y, double z, float radius, bool lastreturn = true);
  int             get_number_vertices();
  virtual TopoClass     get_class() = 0;
  virtual bool          is_hard() = 0;
protected:
  int                   _simplification;
  std::vector<Point3>   _lidarpts;
};


#endif 
