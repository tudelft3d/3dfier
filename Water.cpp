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

#include "Water.h"
#include "io.h"

float Water::_heightref = 0.1;

Water::Water(char *wkt, std::string layername, std::vector<std::tuple<std::string, OGRFieldType, std::string>> attributes, std::string pid, float heightref)
  : Flat(wkt, layername, attributes, pid) {
  _heightref = heightref;
}

TopoClass Water::get_class() {
  return WATER;
}

std::string Water::get_mtl() {
  return "usemtl Water";
}

bool Water::is_hard() {
  return true;
}

bool Water::add_elevation_point(Point2 &p, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  // Add elevation points with radius 0.0 to be inside the water polygon
  if (point_in_polygon(p, *(_p2))) {
    int zcm = int(z * 100);
    //-- 1. assign to polygon since within the threshold value (buffering of polygon)
    _zvaluesinside.push_back(zcm);
  }
  return true;
}

bool Water::lift() {
  Flat::lift_percentile(_heightref);
  return true;
}

std::string Water::get_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<wtr:WaterBody gml:id=\"" << this->get_id() << "\">" << std::endl;
  ss << get_citygml_attributes(_attributes);
  ss << "<wtr:lod1MultiSurface>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << std::setprecision(3) << std::fixed;
  for (auto& t : _triangles)
    ss << get_triangle_as_gml_surfacemember(t);
  for (auto& t : _triangles_vw)
    ss << get_triangle_as_gml_surfacemember(t, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</wtr:lod1MultiSurface>" << std::endl;
  ss << "</wtr:WaterBody>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}

std::string Water::get_citygml_imgeo() {
  bool ondersteunend = _layername == "ondersteunendwaterdeel";
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  if (ondersteunend) {
    ss << "<imgeo:OndersteunendWaterdeel gml:id=\"" << this->get_id() << "\">" << std::endl;
  }
  else {
    ss << "<imgeo:Waterdeel gml:id=\"" << this->get_id() << "\">" << std::endl;
  }
  ss << get_imgeo_object_info(this->get_id());
  ss << "<wtr:lod1MultiSurface>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << std::setprecision(3) << std::fixed;
  for (auto& t : _triangles)
    ss << get_triangle_as_gml_surfacemember(t);
  for (auto& t : _triangles_vw)
    ss << get_triangle_as_gml_surfacemember(t, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</wtr:lod1MultiSurface>" << std::endl;
  std::string attribute;
  if (ondersteunend) {
    if (get_attribute("bgt-type", attribute)) {
      ss << "<wat:class codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOndersteunendWaterdeel\">" << attribute << "</wat:class>" << std::endl;
    }
    if (get_attribute("plus-type", attribute)) {
      ss << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOndersteunendWaterdeelPlus\">" << attribute << "</imgeo:plus-type>" << std::endl;
    }
    ss << "</imgeo:OndersteunendWaterdeel>" << std::endl;
  }
  else {
    if (get_attribute("bgt-type", attribute)) {
      ss << "<wat:class codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeWater\">" << attribute << "</wat:class>" << std::endl;
    }
    if (get_attribute("plus-type", attribute)) {
      ss << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeWaterPlus\">" << attribute << "</imgeo:plus-type>" << std::endl;
    }
    ss << "</imgeo:Waterdeel>" << std::endl;
  }
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}

bool Water::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Water");
}
