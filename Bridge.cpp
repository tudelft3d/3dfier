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

#include "Bridge.h"
#include "io.h"

float Bridge::_heightref = 0.5;

Bridge::Bridge(char *wkt, std::unordered_map<std::string, std::string> attributes, std::string pid, float heightref)
  : Flat(wkt, attributes, pid) {
  _heightref = heightref;
}

TopoClass Bridge::get_class() {
  return BRIDGE;
}

bool Bridge::is_hard() {
  return true;
}

std::string Bridge::get_mtl() {
  return "usemtl Bridge\n";
}

bool Bridge::add_elevation_point(Point2 p, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  if (lastreturn == true && lasclass != LAS_BUILDING && lasclass != LAS_WATER) {
    Flat::add_elevation_point(p, z, radius, lasclass, lastreturn);
  }
  return true;
}

bool Bridge::lift() {
  //lift_each_boundary_vertices(percentile);
  //smooth_boundary(5);
  lift_percentile(_heightref);
  return true;
}

std::string Bridge::get_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<brg:Bridge gml:id=\"";
  ss << this->get_id();
  ss << "\">" << std::endl;
  ss << "<brg:lod1MultiSurface>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << std::setprecision(3) << std::fixed;
  for (auto& t : _triangles)
    ss << get_triangle_as_gml_surfacemember(t);
  for (auto& t : _triangles_vw)
    ss << get_triangle_as_gml_surfacemember(t, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</brg:lod1MultiSurface>" << std::endl;
  ss << "</brg:Bridge>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}

std::string Bridge::get_citygml_imgeo() {
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<bri:BridgeConstructionElement gml:id=\"" << this->get_id() << "\">" << std::endl;
  ss << get_imgeo_object_info(this->get_id());
  ss << "<bri:lod1Geometry>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << std::setprecision(3) << std::fixed;
  for (auto& t : _triangles)
    ss << get_triangle_as_gml_surfacemember(t);
  for (auto& t : _triangles_vw)
    ss << get_triangle_as_gml_surfacemember(t, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</bri:lod1Geometry>" << std::endl;
  std::string attribute;
  //if (get_attribute("bgt_type", attribute)) {
  //  ss << "<imgeo:bgt-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOverbruggingsdeel\">" << attribute << "</imgeo:bgt-type>" << std::endl;
  //}
  if (get_attribute("hoortbijtypeoverbrugging", attribute)) {
    ss << "<imgeo:hoortBijTypeOverbrugging codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOverbrugging\">" << attribute << "</imgeo:hoortBijTypeOverbrugging>" << std::endl;
  }
  if (get_attribute("overbruggingisbeweegbaar", attribute)) {
    ss << "<imgeo:overbruggingIsBeweegbaar>" << attribute << "</imgeo:overbruggingIsBeweegbaar>" << std::endl;
  }
  ss << "</bri:BridgeConstructionElement>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}

bool Bridge::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Bridge");
}
