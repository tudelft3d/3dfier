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

Forest::Forest(char *wkt, std::string layername, std::vector<std::tuple<std::string, OGRFieldType, std::string>> attributes, std::string pid, int simplification, float innerbuffer, bool ground_points_only)
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
  return "usemtl Forest";
}

bool Forest::add_elevation_point(Point2 &p, double z, float radius, LAS14Class lasclass, bool lastreturn) {
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

void Forest::get_citygml(std::ofstream &outputfile) {
  outputfile << "<cityObjectMember>\n";
  outputfile << "<veg:PlantCover gml:id=\"" << this->get_id() << "\">\n";
  get_citygml_attributes(outputfile, _attributes);
  outputfile << "<veg:lod1MultiSurface>\n";
  outputfile << "<gml:MultiSurface>\n";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(outputfile, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(outputfile, t, true);
  outputfile << "</gml:MultiSurface>\n";
  outputfile << "</veg:lod1MultiSurface>\n";
  outputfile << "</veg:PlantCover>\n";
  outputfile << "</cityObjectMember>\n";
}

void Forest::get_citygml_imgeo(std::ofstream &outputfile) {
  outputfile << "<cityObjectMember>\n";
  outputfile << "<veg:PlantCover gml:id=\"" << this->get_id() << "\">\n";
  get_imgeo_object_info(outputfile, this->get_id());
  outputfile << "<veg:lod1MultiSurface>\n";
  outputfile << "<gml:MultiSurface>\n";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(outputfile, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(outputfile, t, true);
  outputfile << "</gml:MultiSurface>\n";
  outputfile << "</veg:lod1MultiSurface>\n";
  std::string attribute;
  if (get_attribute("bgt-fysiekvoorkomen", attribute)) {
    outputfile << "<veg:class codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenBegroeidTerrein\">" << attribute << "</veg:class>\n";
  }
  if (get_attribute("begroeidterreindeeloptalud", attribute, "false")) {
    outputfile << "<imgeo:begroeidTerreindeelOpTalud>" << attribute << "</imgeo:begroeidTerreindeelOpTalud>\n";
  }
  if (get_attribute("plus-fysiekvoorkomen", attribute)) {
    outputfile << "<imgeo:plus-fysiekVoorkomen codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenBegroeidTerreinPlus\">" << attribute << "</imgeo:plus-fysiekVoorkomen>\n";
  }
  outputfile << "</veg:PlantCover>\n";
  outputfile << "</cityObjectMember>\n";
}

bool Forest::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Forest");
}
