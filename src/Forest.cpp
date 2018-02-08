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


#include "Forest.h"
#include "io.h"

bool Forest::_use_ground_points_only = false;

Forest::Forest(char *wkt, std::string layername, AttributeMap attributes, std::string pid, int simplification, double simplification_tinsimp, float innerbuffer, bool ground_points_only)
  : TIN(wkt, layername, attributes, pid, simplification, simplification_tinsimp, innerbuffer)
{
  _use_ground_points_only = ground_points_only;
}

TopoClass Forest::get_class() {
  return FOREST;
}

bool Forest::is_hard() {
  return false;
}

std::string Forest::get_mtl() {
  return "usemtl Forest";
}

bool Forest::add_elevation_point(Point2 &p, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  return TIN::add_elevation_point(p, z, radius, lasclass, lastreturn);
}
//   bool toadd = false;
//   if (lastreturn && ((_use_ground_points_only && lasclass == LAS_GROUND) || (_use_ground_points_only == false && lasclass != LAS_BUILDING))) {
//     toadd = TIN::add_elevation_point(p, z, radius, lasclass, lastreturn);
//   }
//   return toadd;
// }


bool Forest::lift() {
  TopoFeature::lift_each_boundary_vertices(0.5);
  return true;
}


void Forest::get_cityjson(nlohmann::json& j, std::unordered_map<std::string,unsigned long> &dPts) {
  nlohmann::json f;
  f["type"] = "PlantCover";
  f["attributes"];
  get_cityjson_attributes(f, _attributes);
  nlohmann::json g;
  this->get_cityjson_geom(g, dPts);
  f["geometry"].push_back(g);
  j["CityObjects"][this->get_id()] = f;
}


void Forest::get_citygml(std::ostream& of) {
  of << "<cityObjectMember>";
  of << "<veg:PlantCover gml:id=\"" << this->get_id() << "\">";
  get_citygml_attributes(of, _attributes);
  of << "<veg:lod1MultiSurface>";
  of << "<gml:MultiSurface>";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>";
  of << "</veg:lod1MultiSurface>";
  of << "</veg:PlantCover>";
  of << "</cityObjectMember>";
}

void Forest::get_citygml_imgeo(std::ostream& of) {
  of << "<cityObjectMember>";
  of << "<veg:PlantCover gml:id=\"" << this->get_id() << "\">";
  get_imgeo_object_info(of, this->get_id());
  of << "<veg:lod1MultiSurface>";
  of << "<gml:MultiSurface>";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>";
  of << "</veg:lod1MultiSurface>";
  std::string attribute;
  if (get_attribute("bgt-fysiekvoorkomen", attribute)) {
    of << "<veg:class codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenBegroeidTerrein\">" << attribute << "</veg:class>";
  }
  if (get_attribute("begroeidterreindeeloptalud", attribute, "false")) {
    of << "<imgeo:begroeidTerreindeelOpTalud>" << attribute << "</imgeo:begroeidTerreindeelOpTalud>";
  }
  if (get_attribute("plus-fysiekvoorkomen", attribute)) {
    of << "<imgeo:plus-fysiekVoorkomen codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenBegroeidTerreinPlus\">" << attribute << "</imgeo:plus-fysiekVoorkomen>";
  }
  of << "</veg:PlantCover>";
  of << "</cityObjectMember>";
}

bool Forest::get_shape(OGRLayer* layer, bool writeAttributes, AttributeMap extraAttributes) {
  return TopoFeature::get_multipolygon_features(layer, "Forest", writeAttributes, extraAttributes);
}
