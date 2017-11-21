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
  if (lastreturn == true && lasclass != LAS_BUILDING && lasclass != LAS_WATER && lasclass != LAS_BRIDGE) {
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

void Separation::get_citygml(std::wostream& of) {
  of << "<cityObjectMember>";
  of << "<gen:GenericCityObject gml:id=\"" << this->get_id().c_str() << "\">";
  get_citygml_attributes(of, _attributes);
  of << "<gen:lod1Geometry>";
  of << "<gml:MultiSurface>";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>";
  of << "</gen:lod1Geometry>";
  of << "</gen:GenericCityObject>";
  of << "</cityObjectMember>";
}

void Separation::get_citygml_imgeo(std::wostream& of) {
  bool kunstwerkdeel = _layername == "kunstwerkdeel";
  bool overigbouwwerk = _layername == "overigbouwwerk";
  of << "<cityObjectMember>";
  if (kunstwerkdeel) {
    of << "<imgeo:Kunstwerkdeel gml:id=\"" << this->get_id().c_str() << "\">";
  }
  else if (overigbouwwerk) {
    of << "<imgeo:OverigBouwwerk gml:id=\"" << this->get_id().c_str() << "\">";
  }
  else {
    of << "<imgeo:Scheiding gml:id=\"" << this->get_id().c_str() << "\">";
  }
  get_imgeo_object_info(of, this->get_id());
  of << "<imgeo:lod1Geometry>";
  of << "<gml:MultiSurface>";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>";
  of << "</imgeo:lod1Geometry>";
  std::string attribute;
  if (kunstwerkdeel) {
    if (get_attribute("bgt-type", attribute)) {
      of << "<imgeo:bgt-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeKunstwerk\">" << attribute.c_str() << "</imgeo:bgt-type>";
    }
    if (get_attribute("plus-type", attribute)) {
      of << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeKunstwerkPlus\">" << attribute.c_str() << "</imgeo:plus-type>";
    }
    of << "</imgeo:Kunstwerkdeel>";
  }
  else if (overigbouwwerk) {
    if (get_attribute("bgt-type", attribute)) {
      of << "<imgeo:bgt-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOverigBouwwerk\">" << attribute.c_str() << "</imgeo:bgt-type>";
    }
    if (get_attribute("plus-type", attribute)) {
      of << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOverigBouwwerkPlus\">" << attribute.c_str() << "</imgeo:plus-type>";
    }
    of << "</imgeo:OverigBouwwerk>";
  }
  else {
    if (get_attribute("bgt-type", attribute)) {
      of << "<imgeo:bgt-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeScheiding\">" << attribute.c_str() << "</imgeo:bgt-type>";
    }
    if (get_attribute("plus-type", attribute)) {
      of << "<imgeo:plus-type codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeScheidingPlus\">" << attribute.c_str() << "</imgeo:plus-type>";
    }
    of << "</imgeo:Scheiding>";
  }
  of << "</cityObjectMember>";
}

bool Separation::get_shape(OGRLayer* layer, bool writeAttributes, AttributeMap extraAttributes) {
  return TopoFeature::get_multipolygon_features(layer, "Separation", writeAttributes, extraAttributes);
}
