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

#include "Terrain.h"
#include "io.h"
#include <algorithm>

Terrain::Terrain(char *wkt, std::string layername, AttributeMap attributes, std::string pid, int simplification, double simplification_tinsimp, float innerbuffer)
  : TIN(wkt, layername, attributes, pid, simplification, simplification_tinsimp, innerbuffer) {}

TopoClass Terrain::get_class() {
  return TERRAIN;
}

bool Terrain::is_hard() {
  return false;
}

std::string Terrain::get_mtl() {
  return "usemtl Terrain";
}

bool Terrain::add_elevation_point(Point2 &p, double z, float radius, int lasclass, bool within) {
  return TIN::add_elevation_point(p, z, radius, lasclass, within);
}

bool Terrain::lift() {
  //-- lift vertices to their median of lidar points
  TopoFeature::lift_each_boundary_vertices(0.5);
  return true;
}

void Terrain::get_cityjson(nlohmann::json& j, std::unordered_map<std::string,unsigned long> &dPts) {
  nlohmann::json f;
  f["type"] = "LandUse";
  f["attributes"];
  get_cityjson_attributes(f, _attributes);
  nlohmann::json g;
  this->get_cityjson_geom(g, dPts);
  f["geometry"].push_back(g);
  j["CityObjects"][this->get_id()] = f;
}

void Terrain::get_citygml(std::wostream& of) {
  of << "<cityObjectMember>";
  of << "<lu:LandUse gml:id=\"" << this->get_id() << "\">";
  get_citygml_attributes(of, _attributes);
  of << "<lu:lod1MultiSurface>";
  of << "<gml:MultiSurface>";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>";
  of << "</lu:lod1MultiSurface>";
  of << "</lu:LandUse>";
  of << "</cityObjectMember>";
}

void Terrain::get_citygml_imgeo(std::wostream& of) {
  of << "<cityObjectMember>";
  of << "<imgeo:OnbegroeidTerreindeel gml:id=\"" << this->get_id() << "\">";
  get_imgeo_object_info(of, this->get_id());
  of << "<lu:lod1MultiSurface>";
  of << "<gml:MultiSurface>";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>";
  of << "</lu:lod1MultiSurface>";
  std::string attribute;
  if (get_attribute("bgt-fysiekvoorkomen", attribute)) {
    of << "<imgeo:bgt-fysiekVoorkomen codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenOnbegroeidTerrein\">" << attribute /*"erf"*/ << "</imgeo:bgt-fysiekVoorkomen>";
  }
  if (get_attribute("onbegroeidterreindeeloptalud", attribute, "false")) {
    of << "<imgeo:onbegroeidTerreindeelOpTalud>" << attribute << "</imgeo:onbegroeidTerreindeelOpTalud>";
  }
  if (get_attribute("plus-fysiekvoorkomen", attribute)) {
    of << "<imgeo:plus-fysiekVoorkomen codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenOnbegroeidTerreinPlus\">" << attribute << "</imgeo:plus-fysiekVoorkomen>";
  }
  of << "</imgeo:OnbegroeidTerreindeel>";
  of << "</cityObjectMember>";
}

bool Terrain::get_shape(OGRLayer* layer, bool writeAttributes, AttributeMap extraAttributes) {
  return TopoFeature::get_multipolygon_features(layer, "Terrain", writeAttributes, extraAttributes);
}
