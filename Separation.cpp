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

#include "Separation.h"
#include "io.h"

float Separation::_heightref = 0.8;

Separation::Separation(char *wkt, std::string layername, std::vector<std::tuple<std::string, OGRFieldType, std::string>> attributes, std::string pid, float heightref)
  : Flat(wkt, layername, attributes, pid) {
  _heightref = heightref;
}

TopoClass Separation::get_class() {
  return SEPARATION;
}

bool Separation::is_hard() {
  return true;
}

std::string Separation::get_mtl() {
  return "usemtl Separation";
}

bool Separation::add_elevation_point(Point2 p, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  if (lastreturn == true && lasclass != LAS_BUILDING && lasclass != LAS_WATER) {
    Flat::add_elevation_point(p, z, radius, lasclass, lastreturn);
  }
  return true;
}

bool Separation::lift() {
  lift_percentile(_heightref);
  return true;
}

std::string Separation::get_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<gen:GenericCityObject gml:id=\"" << this->get_id() << "\">" << std::endl;
  ss << get_citygml_attributes(_attributes);
  ss << "<gen:lod1Geometry>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << std::setprecision(3) << std::fixed;
  for (auto& t : _triangles)
    ss << get_triangle_as_gml_surfacemember(t);
  for (auto& t : _triangles_vw)
    ss << get_triangle_as_gml_surfacemember(t, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</gen:lod1Geometry>" << std::endl;
  ss << "</gen:GenericCityObject>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}

std::string Separation::get_citygml_imgeo() {
  bool kunstwerkdeel = _layername == "kunstwerkdeel";
  bool overigbouwwerk = _layername == "overigbouwwerk";
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  if (kunstwerkdeel) {
    ss << "<imgeo:Kunstwerkdeel gml:id=\"" << this->get_id() << "\">" << std::endl;
  }
  else if (overigbouwwerk) {
    ss << "<imgeo:OverigBouwwerk gml:id=\"" << this->get_id() << "\">" << std::endl;
  }
  else {
    ss << "<imgeo:Scheiding gml:id=\"" << this->get_id() << "\">" << std::endl;
  }
  ss << get_imgeo_object_info(this->get_id());
  ss << "<imgeo:lod1Geometry>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << std::setprecision(3) << std::fixed;
  for (auto& t : _triangles)
    ss << get_triangle_as_gml_surfacemember(t);
  for (auto& t : _triangles_vw)
    ss << get_triangle_as_gml_surfacemember(t, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</imgeo:lod1Geometry>" << std::endl;
  std::string attribute;
  if (kunstwerkdeel) {
    if (get_attribute("bgt-type", attribute)) {
      ss << "<imgeo:bgt-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeKunstwerk\">" << attribute << "</imgeo:bgt-type>" << std::endl;
    }
    if (get_attribute("plus-type", attribute)) {
      ss << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeKunstwerkPlus\">" << attribute << "</imgeo:plus-type>" << std::endl;
    }
    ss << "</imgeo:Kunstwerkdeel>" << std::endl;
  }
  else if (overigbouwwerk) {
    if (get_attribute("bgt-type", attribute)) {
      ss << "<imgeo:bgt-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOverigBouwwerk\">" << attribute << "</imgeo:bgt-type>" << std::endl;
    }
    if (get_attribute("plus-type", attribute)) {
      ss << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOverigBouwwerkPlus\">" << attribute << "</imgeo:plus-type>" << std::endl;
    }
    ss << "</imgeo:OverigBouwwerk>" << std::endl;
  }
  else {
    if (get_attribute("bgt-type", attribute)) {
      ss << "<imgeo:bgt-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeScheiding\">" << attribute << "</imgeo:bgt-type>" << std::endl;
    }
    if (get_attribute("plus-type", attribute)) {
      ss << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeScheidingPlus\">" << attribute << "</imgeo:plus-type>" << std::endl;
    }
    ss << "</imgeo:Scheiding>" << std::endl;
  }
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}

bool Separation::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Separation");
}
