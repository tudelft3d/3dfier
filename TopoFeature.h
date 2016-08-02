
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
  virtual bool          buildCDT();
  virtual bool          add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn) = 0;
  virtual std::string   get_citygml() = 0;
  virtual std::string   get_obj_v(int z_exaggeration);
  virtual std::string   get_obj_f(int offset, bool usemtl);
  virtual bool          get_shape(OGRLayer*) = 0;
  virtual int           get_number_vertices() = 0;
  virtual TopoClass     get_class() = 0;
  virtual bool          is_hard() = 0;

  std::string  get_id();
  void         construct_vertical_walls(std::vector<TopoFeature*> lsAdj, std::unordered_map< std::string, std::vector<int> > nc);
  void         fix_bowtie(std::vector<TopoFeature*> lsAdj);
  int          get_counter(); 
  Polygon2*    get_Polygon2();
  Box2         get_bbox2d();
  Point2       get_point2(int ringi, int pi);
  bool         has_point2(const Point2& p);
  bool         has_point2_(const Point2& p, std::vector<int>& ringis, std::vector<int>& pis);
  bool         has_segment(Point2& a, Point2& b);
  bool         has_segment(Point2& a, Point2& b, int& aringi, int& api, int& bringi, int& bpi);
  float        get_distance_to_boundaries(Point2& p);
  int          get_vertex_elevation(int ringi, int pi);
  int          get_vertex_elevation(Point2& p);
  void         set_vertex_elevation(int ringi, int pi, int z);  
  void         set_top_level(bool toplevel);
  bool         has_vertical_walls(); 
  void         add_vertical_wall(); 
  bool         get_top_level();
  std::string  get_wkt();
  bool         get_shape_features(OGRLayer* layer, std::string className);

protected:
  Polygon2*                         _p2;
  std::vector< std::vector<int> >   _p2z;
  std::string                       _id;
  int                               _counter;
  static int                        _count;
  bool                              _bVerticalWalls; 
  bool                              _toplevel; 

  //-- used to collect all LiDAR points linked to the polygon
  std::vector< std::vector< std::vector<int> > > _lidarelevs;

  std::set<Point3>   _vertices;  //-- output of Triangle
  std::vector<Triangle> _triangles; //-- output of Triangle
//  std::vector<Point3>   _vertices_vw;  //-- for vertical walls
  std::vector<Triangle> _triangles_vw; //-- for vertical walls

  Point2  get_next_point2_in_ring(int ringi, int& pi);
  bool    assign_elevation_to_vertex(double x, double y, double z, float radius);
  void    lift_each_boundary_vertices(float percentile);
  void    lift_all_boundary_vertices_same_height(int height);
};


//---------------------------------------------

class Flat : public TopoFeature 
{
public:
                      Flat(char *wkt, std::string pid); 
  bool                add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn);
  int                 get_number_vertices();
  int                 get_height();
  virtual bool        lift() = 0;
  virtual std::string get_citygml() = 0;
  virtual TopoClass   get_class() = 0;
  virtual bool        is_hard() = 0;
  // std::string         get_obj_v(int z_exaggeration);
  // std::string         get_obj_f(int offset, bool usemtl);
protected:
  std::vector<int>    _zvaluesinside;
  bool                lift_percentile(float percentile);
};


//---------------------------------------------

class Boundary3D : public TopoFeature
{
public:
                       Boundary3D(char *wkt, std::string pid);
  bool                 add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn);
  int                  get_number_vertices();
  virtual bool         lift() = 0;
  virtual std::string  get_citygml() = 0;
  virtual TopoClass    get_class() = 0;
  virtual bool         is_hard() = 0;
protected:
  int           _simplification;
  void          smooth_boundary(int passes = 1);
};


//---------------------------------------------

class TIN : public TopoFeature
{
public:
                      TIN(char *wkt, std::string pid, int simplification = 0);
  virtual bool        lift() = 0;
  virtual std::string get_citygml() = 0;
  virtual TopoClass   get_class() = 0;
  virtual bool        is_hard() = 0;
  virtual bool        add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn) = 0;
  int                 get_number_vertices();
  bool                buildCDT();
protected:
  int                   _simplification;
  std::vector<Point3>   _lidarpts;
};


#endif 
