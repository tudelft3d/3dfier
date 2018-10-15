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

#include "Water.h"

float Water::_heightref;

Water::Water(char *wkt, std::string layername, AttributeMap attributes, std::string pid, float heightref)
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

bool Water::add_elevation_point(Point2 &p, double z, float radius, int lasclass, bool within) {
  return Flat::add_elevation_point(p, z, radius, lasclass, within);
}

bool Water::lift() {
  Flat::lift_percentile(_heightref);
  return true;
}

void Water::get_cityjson(nlohmann::json& j, std::unordered_map<std::string,unsigned long> &dPts) {
  nlohmann::json f;
  f["type"] = "WaterBody";
  f["attributes"];
  get_cityjson_attributes(f, _attributes);
  nlohmann::json g;
  this->get_cityjson_geom(g, dPts);
  f["geometry"].push_back(g);
  j["CityObjects"][this->get_id()] = f;
}

void Water::get_citygml(std::wostream& of) {
  of << "<cityObjectMember>";
  of << "<wtr:WaterBody gml:id=\"" << this->get_id() << "\">";
  get_citygml_attributes(of, _attributes);
  of << "<wtr:lod1MultiSurface>";
  of << "<gml:MultiSurface>";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>";
  of << "</wtr:lod1MultiSurface>";
  of << "</wtr:WaterBody>";
  of << "</cityObjectMember>";
}

void Water::get_citygml_imgeo(std::wostream& of) {
  bool ondersteunend = _layername == "ondersteunendwaterdeel";
  of << "<cityObjectMember>";
  if (ondersteunend) {
    of << "<imgeo:OndersteunendWaterdeel gml:id=\"" << this->get_id() << "\">";
  }
  else {
    of << "<imgeo:Waterdeel gml:id=\"" << this->get_id() << "\">";
  }
  get_imgeo_object_info(of, this->get_id());
  of << "<wtr:lod1MultiSurface>";
  of << "<gml:MultiSurface>";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>";
  of << "</wtr:lod1MultiSurface>";
  std::string attribute;
  if (ondersteunend) {
    if (get_attribute("bgt-type", attribute)) {
      of << "<wat:class codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOndersteunendWaterdeel\">" << attribute << "</wat:class>";
    }
    else if (get_attribute("bgt_type", attribute)) {
      of << "<wat:class codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOndersteunendWaterdeel\">" << attribute << "</wat:class>";
    }
    if (get_attribute("plus-type", attribute)) {
      of << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOndersteunendWaterdeelPlus\">" << attribute << "</imgeo:plus-type>";
    }
    else if (get_attribute("plus_type", attribute)) {
      of << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOndersteunendWaterdeelPlus\">" << attribute << "</imgeo:plus-type>";
    }
    of << "</imgeo:OndersteunendWaterdeel>";
  }
  else {
    if (get_attribute("bgt-type", attribute)) {
      of << "<wat:class codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeWater\">" << attribute << "</wat:class>";
    }
    else if (get_attribute("bgt_type", attribute)) {
      of << "<wat:class codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeWater\">" << attribute << "</wat:class>";
    }
    if (get_attribute("plus-type", attribute)) {
      of << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeWaterPlus\">" << attribute << "</imgeo:plus-type>";
    }
    else if (get_attribute("plus_type", attribute)) {
      of << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeWaterPlus\">" << attribute << "</imgeo:plus-type>";
    }
    of << "</imgeo:Waterdeel>";
  }
  of << "</cityObjectMember>";
}

bool Water::get_shape(OGRLayer* layer, bool writeAttributes, const AttributeMap& extraAttributes) {
  return TopoFeature::get_multipolygon_features(layer, "Water", writeAttributes, extraAttributes);
}
