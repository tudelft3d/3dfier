/*
  3dfier: takes 2D GIS datasets and "3dfies" to create 3D city models.

  Copyright (C) 2015-2016  3D geoinformation research group, TU Delft

  This file is part of 3dfier.

  3dfier is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  3dfier is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with 3difer.  If not, see <http://www.gnu.org/licenses/>.

  For any information or further details about the use of 3dfier, contact
  Hugo Ledoux
  <h.ledoux@tudelft.nl>
  Faculty of Architecture & the Built Environment
  Delft University of Technology
  Julianalaan 134, Delft 2628BL, the Netherlands
*/

#ifndef __3DFIER__TopoFeature__
#define __3DFIER__TopoFeature__

#include "definitions.h"
#include "geomtools.h"
#include <random>

class TopoFeature {
public:
  TopoFeature(char *wkt, std::string layername, std::unordered_map<std::string, std::string> attributes, std::string pid);
  ~TopoFeature();

  virtual bool          lift() = 0;
  virtual bool          buildCDT();
  virtual bool          add_elevation_point(Point2 p, double z, float radius, LAS14Class lasclass, bool lastreturn) = 0;
  virtual int           get_number_vertices() = 0;
  virtual TopoClass     get_class() = 0;
  virtual bool          is_hard() = 0;
  virtual std::string   get_mtl() = 0;
  virtual std::string   get_citygml() = 0;
  virtual std::string   get_citygml_imgeo() = 0;
  virtual bool          get_shape(OGRLayer*) = 0;

  std::string  get_id();
  void         construct_vertical_walls(std::unordered_map< std::string, std::vector<int> > &nc);
  void         fix_bowtie();
  void         add_adjacent_feature(TopoFeature* adjFeature);
  std::vector<TopoFeature*>* get_adjacent_features();
  int          get_counter();
  Polygon2*    get_Polygon2();
  Box2         get_bbox2d();
  Point2       get_point2(int ringi, int pi);
  bool         has_point2(const Point2& p);
  bool         has_point2_(const Point2& p, std::vector<int>& ringis, std::vector<int>& pis);
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
  std::string  get_obj(std::unordered_map< std::string, unsigned long > &dPts);
  std::string  get_imgeo_object_info(std::string id);
protected:
  Polygon2*                         _p2;
  std::vector< std::vector<int> >   _p2z;
  std::vector<TopoFeature*>*        _adjFeatures;
  std::string                       _id;
  int                               _counter;
  static int                        _count;
  bool                              _bVerticalWalls;
  bool                              _toplevel;
  std::string                       _layername;
  std::unordered_map<std::string, std::string> _attributes;

  std::vector< std::vector< std::vector<int> > > _lidarelevs; //-- used to collect all LiDAR points linked to the polygon
  std::vector<Point3>   _vertices;  //-- output of Triangle
  std::vector<Triangle> _triangles; //-- output of Triangle
  std::vector<Point3>   _vertices_vw;  //-- for vertical walls
  std::vector<Triangle> _triangles_vw; //-- for vertical walls

  Point2  get_next_point2_in_ring(int ringi, int i, int& pi);
  bool    assign_elevation_to_vertex(Point2 p, double z, float radius);
  void    lift_each_boundary_vertices(float percentile);
  void    lift_all_boundary_vertices_same_height(int height);

  std::string get_triangle_as_gml_surfacemember(Triangle& t, bool verticalwall = false);
  std::string get_triangle_as_gml_triangle(Triangle& t, bool verticalwall = false);
  bool get_attribute(std::string attributeName, std::string &attribute, std::string defaultValue = "");
};

//---------------------------------------------

class Flat: public TopoFeature {
public:
  Flat(char *wkt, std::string layername, std::unordered_map<std::string, std::string> attributes, std::string pid);
  int                 get_number_vertices();
  bool                add_elevation_point(Point2 p, double z, float radius, LAS14Class lasclass, bool lastreturn);
  int                 get_height();
  virtual TopoClass   get_class() = 0;
  virtual bool        is_hard() = 0;
  virtual bool        lift() = 0;
  virtual std::string get_citygml() = 0;
protected:
  std::vector<int>    _zvaluesinside;
  bool                lift_percentile(float percentile);
};

//---------------------------------------------

class Boundary3D: public TopoFeature {
public:
  Boundary3D(char *wkt, std::string layername, std::unordered_map<std::string, std::string> attributes, std::string pid);
  int                  get_number_vertices();
  bool                 add_elevation_point(Point2 p, double z, float radius, LAS14Class lasclass, bool lastreturn);
  virtual TopoClass    get_class() = 0;
  virtual bool         is_hard() = 0;
  virtual bool         lift() = 0;
  virtual std::string  get_citygml() = 0;
protected:
  int                  _simplification;
  void                 smooth_boundary(int passes = 1);
};

//---------------------------------------------

class TIN: public TopoFeature {
public:
  TIN(char *wkt, std::string layername, std::unordered_map<std::string, std::string> attributes, std::string pid, int simplification = 0, float innerbuffer = 0);
  int                 get_number_vertices();
  bool                add_elevation_point(Point2 p, double z, float radius, LAS14Class lasclass, bool lastreturn);
  virtual TopoClass   get_class() = 0;
  virtual bool        is_hard() = 0;
  virtual bool        lift() = 0;
  virtual std::string get_citygml() = 0;
  bool                buildCDT();
protected:
  int                 _simplification;
  float               _innerbuffer;
  std::vector<Point3> _lidarpts;
};

#endif 
