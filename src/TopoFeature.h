/*
  3dfier: takes 2D GIS datasets and "3dfies" to create 3D city models.

  Copyright (C) 2015-2018  3D geoinformation research group, TU Delft

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
#include "nlohmann-json/json.hpp"
#include <random>

class TopoFeature {
public:
  TopoFeature(char *wkt, std::string layername, AttributeMap attributes, std::string pid);
  ~TopoFeature();

  virtual bool          lift() = 0;
  virtual bool          buildCDT();
  virtual bool          add_elevation_point(Point2 &p, double z, float radius, int lasclass) = 0;
  virtual int           get_number_vertices() = 0;
  virtual TopoClass     get_class() = 0;
  virtual bool          is_hard() = 0;
  virtual std::string   get_mtl() = 0;
  virtual void          get_citygml(std::wostream& of) = 0;
  virtual void          get_cityjson(nlohmann::json& j, std::unordered_map<std::string, unsigned long> &dPts) = 0;
  virtual void          get_citygml_imgeo(std::wostream& of) = 0;
  virtual bool          get_shape(OGRLayer*, bool writeAttributes, AttributeMap extraAttributes = AttributeMap()) = 0;

  std::string  get_id();
  void         construct_vertical_walls(const NodeColumn& nc);
  void         fix_bowtie();
  void         add_adjacent_feature(TopoFeature* adjFeature);
  std::vector<TopoFeature*>* get_adjacent_features();
  Polygon2*    get_Polygon2();
  Box2         get_bbox2d();
  std::string  get_layername();
  Point2       get_point2(int ringi, int pi);
  bool         has_point2(const Point2& p, std::vector<int>& ringis, std::vector<int>& pis);
  bool         has_segment(Point2& a, Point2& b, int& aringi, int& api, int& bringi, int& bpi);
  bool         adjacent(const Polygon2& poly);
  float        get_distance_to_boundaries(Point2& p);
  int          get_vertex_elevation(int ringi, int pi);
  int          get_vertex_elevation(Point2& p);
  void         set_vertex_elevation(int ringi, int pi, int z);
  void         set_top_level(bool toplevel);
  bool         has_vertical_walls();
  void         add_vertical_wall();
  bool         get_top_level();
  bool         get_multipolygon_features(OGRLayer* layer, std::string className, bool writeAttributes, AttributeMap extraAttributes = AttributeMap(), bool writeHeights = false, int height_base = 0, int height = 0);
  void         get_obj(std::unordered_map< std::string, unsigned long > &dPts, std::string mtl, std::string &fs);
  AttributeMap get_attributes();
  void         get_imgeo_object_info(std::wostream& of, std::string id);
  void         get_citygml_attributes(std::wostream& of, AttributeMap attributes);
  void         get_cityjson_attributes(nlohmann::json& f, AttributeMap attributes);
protected:
  Polygon2*                         _p2;
  std::vector< std::vector<int> >   _p2z;
  std::vector<TopoFeature*>*        _adjFeatures;
  std::string                       _id;
  bool                              _bVerticalWalls;
  bool                              _toplevel;
  std::string                       _layername;
  AttributeMap                      _attributes;

  std::vector< std::vector< std::vector<int> > >  _lidarelevs; //-- used to collect all LiDAR points linked to the polygon
  std::vector< std::pair<Point3, std::string> >   _vertices;
  std::vector<Triangle>                           _triangles;
  std::vector< std::pair<Point3, std::string> >   _vertices_vw;
  std::vector<Triangle>                           _triangles_vw;

  Point2  get_next_point2_in_ring(int ringi, int i, int& pi);
  bool    assign_elevation_to_vertex(Point2 &p, double z, float radius);
  bool    within_range(Point2 &p, Polygon2 &oly, double radius);
  bool    point_in_polygon(const Point2 &p, const Polygon2 &poly);
  void    lift_each_boundary_vertices(float percentile);
  void    lift_all_boundary_vertices_same_height(int height);

  void get_cityjson_geom(nlohmann::json& g, std::unordered_map<std::string, unsigned long> &dPts, std::string primitive = "MultiSurface");
  void get_triangle_as_gml_surfacemember(std::wostream& of, Triangle& t, bool verticalwall = false);
  void get_floor_triangle_as_gml_surfacemember(std::wostream& of, Triangle& t, int baseheight);
  void get_triangle_as_gml_triangle(std::wostream& of, Triangle& t, bool verticalwall = false);
  bool get_attribute(std::string attributeName, std::string &attribute, std::string defaultValue = "");
};

//---------------------------------------------

class Flat: public TopoFeature {
public:
  Flat(char *wkt, std::string layername, AttributeMap attributes, std::string pid);
  int                 get_number_vertices();
  bool                add_elevation_point(Point2 &p, double z, float radius, int lasclass);
  int                 get_height();
  virtual TopoClass   get_class() = 0;
  virtual bool        is_hard() = 0;
  virtual bool        lift() = 0;
  virtual void        get_citygml(std::wostream& of) = 0;
  virtual void        get_cityjson(nlohmann::json& j, std::unordered_map<std::string, unsigned long> &dPts) = 0;
protected:
  std::vector<int>    _zvaluesinside;
  bool                lift_percentile(float percentile);
};

//---------------------------------------------

class Boundary3D: public TopoFeature {
public:
  Boundary3D(char *wkt, std::string layername, AttributeMap attributes, std::string pid);
  int                  get_number_vertices();
  bool                 add_elevation_point(Point2 &p, double z, float radius, int lasclass);
  virtual TopoClass    get_class() = 0;
  virtual bool         is_hard() = 0;
  virtual bool         lift() = 0;
  virtual void         get_citygml(std::wostream& of) = 0;
  virtual void         get_cityjson(nlohmann::json& j, std::unordered_map<std::string, unsigned long> &dPts) = 0;
  void                 detect_outliers(bool replace_all);
protected:
  int                  _simplification;
  void                 smooth_boundary(int passes = 1);
};

//---------------------------------------------

class TIN: public TopoFeature {
public:
  TIN(char *wkt, std::string layername, AttributeMap attributes, std::string pid, int simplification = 0, double simplification_tinsimp = 0, float innerbuffer = 0);
  int                 get_number_vertices();
  bool                add_elevation_point(Point2 &p, double z, float radius, int lasclass);
  virtual TopoClass   get_class() = 0;
  virtual bool        is_hard() = 0;
  virtual bool        lift() = 0;
  virtual void        get_citygml(std::wostream& of) = 0;
  virtual void        get_cityjson(nlohmann::json& j, std::unordered_map<std::string, unsigned long> &dPts) = 0;
  bool                buildCDT();
protected:
  int                 _simplification;
  double              _simplification_tinsimp;
  float               _innerbuffer;
  std::vector<Point3> _lidarpts;
};

#endif 
