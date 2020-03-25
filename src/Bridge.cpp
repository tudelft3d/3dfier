/*
  3dfier: takes 2D GIS datasets and "3dfies" to create 3D city models.

  Copyright (C) 2015-2020 3D geoinformation research group, TU Delft

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

#include "Bridge.h"

float Bridge::_heightref;
bool Bridge::_flatten;

Bridge::Bridge(char *wkt, std::string layername, AttributeMap attributes, std::string pid, float heightref, bool flatten)
  : Boundary3D(wkt, layername, attributes, pid) {
  _heightref = heightref;
  _flatten = flatten;
}

TopoClass Bridge::get_class() {
  return BRIDGE;
}

bool Bridge::is_hard() {
  return true;
}

void Bridge::cleanup_elevations() {
  TopoFeature::cleanup_lidarelevs();
}

std::string Bridge::get_mtl() {
  return "usemtl Bridge";
}

bool Bridge::add_elevation_point(Point2 &p, double z, float radius, int lasclass, bool within) {
  return Boundary3D::add_elevation_point(p, z, radius, lasclass, within);
}

bool Bridge::push_distance(double dist) {
  _distancesinside[0].push_back(dist);
  return true;
}

void Bridge::clear_distances() {
  if (!_distancesinside[0].empty()) { _distancesinside[0].clear(); }
}

bool Bridge::lift() {
  lift_each_boundary_vertices(_heightref);
  return true;
}

bool Bridge::get_flatten() {
  return _flatten;
}

void Bridge::get_cityjson(nlohmann::json& j, std::unordered_map<std::string,unsigned long> &dPts) {
  nlohmann::json f;
  f["type"] = "Bridge"; 
  f["attributes"];
  get_cityjson_attributes(f, _attributes);
  nlohmann::json g;
  this->get_cityjson_geom(g, dPts);
  f["geometry"].push_back(g);
  j["CityObjects"][this->get_id()] = f;
}

void Bridge::get_citygml(std::wostream& of) {
  of << "<cityObjectMember>";
  of << "<bri:Bridge gml:id=\"" << this->get_id() << "\">";
  get_citygml_attributes(of, _attributes);
  of << "<bri:lod1MultiSurface>";
  of << "<gml:MultiSurface>";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>";
  of << "</bri:lod1MultiSurface>";
  of << "</bri:Bridge>";
  of << "</cityObjectMember>";
}

void Bridge::get_citygml_imgeo(std::wostream& of) {
  of << "<cityObjectMember>";
  of << "<bri:BridgeConstructionElement gml:id=\"" << this->get_id() << "\">";
  get_imgeo_attributes(of, this->get_id());
  of << "<bri:lod1Geometry>";
  of << "<gml:MultiSurface>";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>";
  of << "</bri:lod1Geometry>";
  std::string attribute;
  if (get_attribute("bgt-type", attribute)) {
    of << "<bri:function codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOverbruggingsdeel\">" << attribute << "</bri:function>";
  }  
  else if (get_attribute("bgt_type", attribute)) {
    of << "<bri:function codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOverbruggingsdeel\">" << attribute << "</bri:function>";
  }
  if (get_attribute("hoortbijtypeoverbrugging", attribute)) {
    of << "<imgeo:hoortBijTypeOverbrugging codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOverbrugging\">" << attribute << "</imgeo:hoortBijTypeOverbrugging>";
  }
  if (get_attribute("overbruggingisbeweegbaar", attribute)) {
    of << "<imgeo:overbruggingIsBeweegbaar>" << attribute << "</imgeo:overbruggingIsBeweegbaar>";
  }
  of << "</bri:BridgeConstructionElement>";
  of << "</cityObjectMember>";
}

bool Bridge::get_shape(OGRLayer* layer, bool writeAttributes, const AttributeMap& extraAttributes) {
  return TopoFeature::get_multipolygon_features(layer, "Bridge", writeAttributes, extraAttributes);
}
