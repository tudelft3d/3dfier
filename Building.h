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

#ifndef Building_h
#define Building_h

#include "TopoFeature.h"

class Building: public Flat {
public:
  Building(char *wkt, std::string layername, std::vector<std::tuple<std::string, OGRFieldType, std::string>> attributes, std::string pid, float heightref_top, float heightref_base);
  bool          lift();
  bool          add_elevation_point(Point2 &p, double z, float radius, LAS14Class lasclass, bool lastreturn);
  void          get_obj(std::unordered_map< std::string, unsigned long > &dPts, int lod, std::string mtl, std::ostringstream &ssf);
  std::string   get_citygml();
  std::string   get_citygml_imgeo();
  std::string   get_imgeo_nummeraanduiding();
  std::string   get_csv();
  std::string   get_mtl();
  bool          get_shape(OGRLayer * layer);
  TopoClass     get_class();
  bool          is_hard();
  int           get_height_base();
private:
  std::vector<int>    _zvaluesground;
  static float        _heightref_top;
  static float        _heightref_base;
  int                 _height_base;
};

#endif /* Building_h */
