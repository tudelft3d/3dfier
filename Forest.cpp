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

Forest::Forest(char *wkt, std::string layername, std::unordered_map<std::string, std::string> attributes, std::string pid, int simplification, float innerbuffer, bool ground_points_only)
  : TIN(wkt, layername, attributes, pid, simplification, innerbuffer)
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
  return "usemtl Forest\n";
}

bool Forest::add_elevation_point(Point2 p, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  bool toadd = false;
  if (lastreturn && ((_use_ground_points_only && lasclass == LAS_GROUND) || (_use_ground_points_only == false && lasclass != LAS_BUILDING))) {
    toadd = TIN::add_elevation_point(p, z, radius, lasclass, lastreturn);
  }
  return toadd;
}

bool Forest::lift() {
  TopoFeature::lift_each_boundary_vertices(0.5);
  return true;
}

std::string Forest::get_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<veg:PlantCover gml:id=\"";
  ss << this->get_id();
  ss << "\">" << std::endl;
  ss << "<veg:lod1MultiSurface>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << std::setprecision(3) << std::fixed;
  for (auto& t : _triangles)
    ss << get_triangle_as_gml_surfacemember(t);
  for (auto& t : _triangles_vw)
    ss << get_triangle_as_gml_surfacemember(t, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</veg:lod1MultiSurface>" << std::endl;
  ss << "</veg:PlantCover>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}

std::string Forest::get_citygml_imgeo() {
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<veg:PlantCover gml:id=\"" << this->get_id() << "\">" << std::endl;
  ss << get_imgeo_object_info(this->get_id());
  ss << "<veg:lod1MultiSurface>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << std::setprecision(3) << std::fixed;
  for (auto& t : _triangles)
    ss << get_triangle_as_gml_surfacemember(t);
  for (auto& t : _triangles_vw)
    ss << get_triangle_as_gml_surfacemember(t, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</veg:lod1MultiSurface>" << std::endl;
  std::string attribute;
  if (get_attribute("bgt_fysiekvoorkomen", attribute)) {
    ss << "<veg:class codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenBegroeidTerrein\">" << attribute << "</veg:class>" << std::endl;
  }
  if (get_attribute("begroeidterreindeeloptalud", attribute, "false")) {
    ss << "<imgeo:begroeidTerreindeelOpTalud>" << attribute << "</imgeo:begroeidTerreindeelOpTalud>" << std::endl;
  }
  if (get_attribute("plus_fysiekvoorkomen", attribute)) {
    ss << "<imgeo:plus-fysiekVoorkomen codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenBegroeidTerreinPlus\">" << attribute << "</imgeo:plus-fysiekVoorkomen>" << std::endl;
  }
  ss << "</veg:PlantCover>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}

bool Forest::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Forest");
}
