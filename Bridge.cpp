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

Bridge::Bridge(char *wkt, std::string layername, std::vector<std::tuple<std::string, OGRFieldType, std::string>> attributes, std::string pid, float heightref)
  : Flat(wkt, layername, attributes, pid) {
  _heightref = heightref;
}

TopoClass Bridge::get_class() {
  return BRIDGE;
}

bool Bridge::is_hard() {
  return true;
}

std::string Bridge::get_mtl() {
  return "usemtl Bridge";
}

bool Bridge::add_elevation_point(Point2 &p, double z, float radius, LAS14Class lasclass, bool lastreturn) {
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

void Bridge::get_citygml(std::ofstream &outputfile) {
  outputfile << "<cityObjectMember>\n";
  outputfile << "<brg:Bridge gml:id=\"" << this->get_id() << "\">\n";
  get_citygml_attributes(outputfile, _attributes);
  outputfile << "<brg:lod1MultiSurface>\n";
  outputfile << "<gml:MultiSurface>\n";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(outputfile, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(outputfile, t, true);
  outputfile << "</gml:MultiSurface>\n";
  outputfile << "</brg:lod1MultiSurface>\n";
  outputfile << "</brg:Bridge>\n";
  outputfile << "</cityObjectMember>\n";
}

void Bridge::get_citygml_imgeo(std::ofstream &outputfile) {
  outputfile << "<cityObjectMember>\n";
  outputfile << "<bri:BridgeConstructionElement gml:id=\"" << this->get_id() << "\">\n";
  get_imgeo_object_info(outputfile, this->get_id());
  outputfile << "<bri:lod1Geometry>\n";
  outputfile << "<gml:MultiSurface>\n";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(outputfile, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(outputfile, t, true);
  outputfile << "</gml:MultiSurface>\n";
  outputfile << "</bri:lod1Geometry>\n";
  std::string attribute;
  if (get_attribute("bgt-type", attribute)) {
    outputfile << "<bri:function codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOverbruggingsdeel\">" << attribute << "</bri:function>\n";
  }
  if (get_attribute("hoortbijtypeoverbrugging", attribute)) {
    outputfile << "<imgeo:hoortBijTypeOverbrugging codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOverbrugging\">" << attribute << "</imgeo:hoortBijTypeOverbrugging>\n";
  }
  if (get_attribute("overbruggingisbeweegbaar", attribute)) {
    outputfile << "<imgeo:overbruggingIsBeweegbaar>" << attribute << "</imgeo:overbruggingIsBeweegbaar>\n";
  }
  outputfile << "</bri:BridgeConstructionElement>\n";
  outputfile << "</cityObjectMember>\n";
}

bool Bridge::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Bridge");
}
