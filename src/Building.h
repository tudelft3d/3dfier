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

#ifndef Building_h
#define Building_h

#include "TopoFeature.h"

class Building: public Flat {
public:
  Building(char *wkt, std::string layername, AttributeMap attributes, std::string pid, float heightref_top, float heightref_base);
  bool          lift();
  bool          add_elevation_point(Point2 &p, double z, float radius, int lasclass);
  void          get_obj(std::unordered_map< std::string, unsigned long > &dPts, int lod, std::string mtl, std::string &fs);
  void          get_citygml(std::ostream& of);
  void          get_citygml_imgeo(std::ostream& of);
  void          get_imgeo_nummeraanduiding(std::ostream& of);
  void          get_csv(std::ostream& of);
  void          get_cityjson(nlohmann::json& j, std::unordered_map<std::string, unsigned long> &dPts);
  std::string   get_all_z_values();
  std::string   get_mtl();
  bool          get_shape(OGRLayer* layer, bool writeAttributes, AttributeMap extraAttributes = AttributeMap());
  TopoClass     get_class();
  bool          is_hard();
  int           get_height_base();
  int           get_height_ground_at_percentile(float percentile);
  int           get_height_roof_at_percentile(float percentile);

  static void   set_las_classes_roof(std::set<int> theset);
  static void   set_las_classes_ground(std::set<int> theset);
private:
  std::vector<int>     _zvaluesground;
  int                  _height_base;
  static float         _heightref_top;
  static float         _heightref_base;
  static std::set<int> _las_classes_roof;
  static std::set<int> _las_classes_ground;
};

#endif /* Building_h */
