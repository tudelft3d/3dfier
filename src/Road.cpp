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

#include "Road.h"
#include "io.h"

float Road::_heightref;
int Road::_threshold_outliers;

Road::Road(char *wkt, std::string layername, AttributeMap attributes, std::string pid, float heightref, int threshold_outliers)
  : Boundary3D(wkt, layername, attributes, pid) {
  _heightref = heightref;
  _threshold_outliers = threshold_outliers;
}

TopoClass Road::get_class() {
  return ROAD;
}

bool Road::is_hard() {
  return true;
}

std::string Road::get_mtl() {
  return "usemtl Road";
}

bool Road::add_elevation_point(Point2 &p, double z, float radius, int lasclass) {
  Boundary3D::add_elevation_point(p, z, radius, lasclass);
  return true;
}

bool Road::push_distance(double dist, int lasclass) {
  if (_distancesinside.size()==0) { _distancesinside.resize(8); }
  _distancesinside[0].push_back(dist);
  return true;
}


bool Road::lift() {
  lift_each_boundary_vertices(_heightref);
  //smooth_boundary(5);
  detect_outliers(_threshold_outliers);
  return true;
}

void Road::get_cityjson(nlohmann::json& j, std::unordered_map<std::string,unsigned long> &dPts) {
  nlohmann::json f;
  f["type"] = "Road";
  f["attributes"];
  get_cityjson_attributes(f, _attributes);
  nlohmann::json g;
  this->get_cityjson_geom(g, dPts);
  f["geometry"].push_back(g);
  j["CityObjects"][this->get_id()] = f;
}

void Road::get_citygml(std::wostream& of) {
  of << "<cityObjectMember>";
  of << "<tra:Road gml:id=\"" << this->get_id() << "\">";
  get_citygml_attributes(of, _attributes);
  of << "<tra:lod1MultiSurface>";
  of << "<gml:MultiSurface>";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>";
  of << "</tra:lod1MultiSurface>";
  of << "</tra:Road>";
  of << "</cityObjectMember>";
}

void Road::get_citygml_imgeo(std::wostream& of) {
  bool auxiliary = _layername == "auxiliarytrafficarea";
  bool spoor = _layername == "spoor";
  of << "<cityObjectMember>";
  if (spoor) {
    of << "<tra:Railway gml:id=\"" << this->get_id() << "\">";
  }
  else if (auxiliary) {
    of << "<tra:AuxiliaryTrafficArea gml:id=\"" << this->get_id() << "\">";
  }
  else {
    of << "<tra:TrafficArea gml:id=\"" << this->get_id() << "\">";
  }
  get_imgeo_object_info(of, this->get_id());
  of << "<tra:lod2MultiSurface>";
  of << "<gml:MultiSurface>";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(of, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(of, t, true);
  of << "</gml:MultiSurface>";
  of << "</tra:lod2MultiSurface>";
  std::string attribute;

  if (spoor) {
    if (get_attribute("bgt-functie", attribute)) {
      of << "<tra:function codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FunctieSpoor\">" << attribute << "</tra:function>";
    }
    if (get_attribute("plus-functiespoor", attribute)) {
      of << "<imgeo:plus-functieSpoor codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FunctieSpoorPlus\">" << attribute << "</imgeo:plus-functieSpoor>";
    }
    of << "</tra:Railway>";
  }
  else if (auxiliary) {
    if (get_attribute("bgt-functie", attribute)) {
      of << "<tra:function codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOndersteunendWegdeel\">" << attribute << "</tra:function>";
    }
    if (get_attribute("bgt-fysiekvoorkomen", attribute)) {
      of << "<tra:surfaceMaterial codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenOndersteunendWegdeel\">" << attribute << "</imgeo:tra:surfaceMaterial>";
    }
    if (get_attribute("ondersteunendwegdeeloptalud", attribute, "false")) {
      of << "<imgeo:ondersteunendWegdeelOpTalud>" << attribute << "</imgeo:ondersteunendWegdeelOpTalud>";
    }
    if (get_attribute("plus-functieondersteunendwegdeel", attribute)) {
      of << "<imgeo:plus-functieOndersteunendWegdeel codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOndersteunendWegdeelPlus\">" << attribute << "</imgeo:plus-functieOndersteunendWegdeel>";
    }
    if (get_attribute("plus-fysiekvoorkomenondersteunendwegdeel", attribute)) {
      of << "<imgeo:plus-fysiekVoorkomenOndersteunendWegdeel codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenOndersteunendWegdeelPlus\">" << attribute << "</imgeo:plus-fysiekVoorkomenOndersteunendWegdeel>";
    }
    of << "</tra:AuxiliaryTrafficArea>";
  }
  else
  {
    if (get_attribute("bgt-functie", attribute)) {
      of << "<tra:function codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FunctieWeg\">" << attribute << "</tra:function>";
    }
    if (get_attribute("bgt-fysiekvoorkomen", attribute)) {
      of << "<tra:surfaceMaterial codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenWeg\">" << attribute << "</tra:surfaceMaterial>";
    }
    if (!get_attribute("wegdeeloptalud", attribute, "false")) {
      of << "<imgeo:wegdeelOpTalud>" << attribute << "</imgeo:wegdeelOpTalud>";
    }
    if (get_attribute("plus-functiewegdeel", attribute)) {
      of << "<imgeo:plus-functieWegdeel codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FunctieWegPlus\">" << attribute << "</imgeo:plus-functieWegdeel>";
    }
    if (get_attribute("plus-fysiekvoorkomenwegdeel", attribute)) {
      of << "<imgeo:plus-fysiekVoorkomenWegdeel codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenWegPlus\">" << attribute << "</imgeo:plus-fysiekVoorkomenWegdeel>";
    }
    of << "</tra:TrafficArea>";
  }
  of << "</cityObjectMember>";
}

bool Road::get_shape(OGRLayer* layer, bool writeAttributes, AttributeMap extraAttributes) {
  return TopoFeature::get_multipolygon_features(layer, "Road", writeAttributes, extraAttributes);
}
