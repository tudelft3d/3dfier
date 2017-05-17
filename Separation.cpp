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

float Separation::_heightref = 0.8f;

Separation::Separation(char *wkt, std::string layername, AttributeMap attributes, std::string pid, float heightref)
  : Boundary3D(wkt, layername, attributes, pid) {
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

bool Separation::add_elevation_point(Point2 &p, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  if (lastreturn == true && lasclass != LAS_BUILDING && lasclass != LAS_WATER) {
    Boundary3D::add_elevation_point(p, z, radius, lasclass, lastreturn);
  }
  return true;
}

bool Separation::lift() {
  //lift_percentile(_heightref);
  //return true;
  lift_each_boundary_vertices(_heightref);
  smooth_boundary(5);
  return true;
}

void Separation::get_citygml(std::ofstream& of) {
  of << "<cityObjectMember>\n";
  of << "<gen:GenericCityObject gml:id=\"" << this->get_id() << "\">\n";
  get_citygml_attributes(of, _attributes);
  of << "<gen:lod1Geometry>\n";
  of << "<gml:MultiSurface>\n";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>\n";
  of << "</gen:lod1Geometry>\n";
  of << "</gen:GenericCityObject>\n";
  of << "</cityObjectMember>\n";
}

void Separation::get_citygml_imgeo(std::ofstream& of) {
  bool kunstwerkdeel = _layername == "kunstwerkdeel";
  bool overigbouwwerk = _layername == "overigbouwwerk";
  of << "<cityObjectMember>\n";
  if (kunstwerkdeel) {
    of << "<imgeo:Kunstwerkdeel gml:id=\"" << this->get_id() << "\">\n";
  }
  else if (overigbouwwerk) {
    of << "<imgeo:OverigBouwwerk gml:id=\"" << this->get_id() << "\">\n";
  }
  else {
    of << "<imgeo:Scheiding gml:id=\"" << this->get_id() << "\">\n";
  }
  get_imgeo_object_info(of, this->get_id());
  of << "<imgeo:lod1Geometry>\n";
  of << "<gml:MultiSurface>\n";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>\n";
  of << "</imgeo:lod1Geometry>\n";
  std::string attribute;
  if (kunstwerkdeel) {
    if (get_attribute("bgt-type", attribute)) {
      of << "<imgeo:bgt-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeKunstwerk\">" << attribute << "</imgeo:bgt-type>\n";
    }
    if (get_attribute("plus-type", attribute)) {
      of << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeKunstwerkPlus\">" << attribute << "</imgeo:plus-type>\n";
    }
    of << "</imgeo:Kunstwerkdeel>\n";
  }
  else if (overigbouwwerk) {
    if (get_attribute("bgt-type", attribute)) {
      of << "<imgeo:bgt-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOverigBouwwerk\">" << attribute << "</imgeo:bgt-type>\n";
    }
    if (get_attribute("plus-type", attribute)) {
      of << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOverigBouwwerkPlus\">" << attribute << "</imgeo:plus-type>\n";
    }
    of << "</imgeo:OverigBouwwerk>\n";
  }
  else {
    if (get_attribute("bgt-type", attribute)) {
      of << "<imgeo:bgt-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeScheiding\">" << attribute << "</imgeo:bgt-type>\n";
    }
    if (get_attribute("plus-type", attribute)) {
      of << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeScheidingPlus\">" << attribute << "</imgeo:plus-type>\n";
    }
    of << "</imgeo:Scheiding>\n";
  }
  of << "</cityObjectMember>\n";
}

bool Separation::get_shape(OGRLayer* layer, bool writeAttributes) {
  return TopoFeature::get_multipolygon_features(layer, "Separation", writeAttributes);
}
