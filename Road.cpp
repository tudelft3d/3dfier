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

#include "Road.h"
#include "io.h"

float Road::_heightref = 0.5;

Road::Road(char *wkt, std::string layername, std::unordered_map<std::string, std::pair<OGRFieldType, std::string>> attributes, std::string pid, float heightref)
  : Boundary3D(wkt, layername, attributes, pid) {
  _heightref = heightref;
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

bool Road::add_elevation_point(Point2 &p, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  if (lastreturn == true && lasclass == LAS_GROUND) {
    Boundary3D::add_elevation_point(p, z, radius, lasclass, lastreturn);
  }
  return true;
}

bool Road::lift() {
  lift_each_boundary_vertices(_heightref);
  smooth_boundary(5);
  return true;
}

void Road::get_citygml(std::ofstream &outputfile) {
  outputfile << "<cityObjectMember>\n";
  outputfile << "<tran:Road gml:id=\"" << this->get_id() << "\">\n";
  get_citygml_attributes(outputfile, _attributes);
  outputfile << "<tran:lod1MultiSurface>\n";
  outputfile << "<gml:MultiSurface>\n";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(outputfile, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(outputfile, t, true);
  outputfile << "</gml:MultiSurface>\n";
  outputfile << "</tran:lod1MultiSurface>\n";
  outputfile << "</tran:Road>\n";
  outputfile << "</cityObjectMember>\n";
}

void Road::get_citygml_imgeo(std::ofstream &outputfile) {
  bool auxiliary = _layername == "auxiliarytrafficarea";
  bool spoor = _layername == "spoor";
  outputfile << "<cityObjectMember>\n";
  if (spoor) {
    outputfile << "<tra:Railway gml:id=\"" << this->get_id() << "\">\n";
  }
  else if (auxiliary) {
    outputfile << "<tra:AuxiliaryTrafficArea gml:id=\"" << this->get_id() << "\">\n";
  }
  else {
    outputfile << "<tra:TrafficArea gml:id=\"" << this->get_id() << "\">\n";
  }
  get_imgeo_object_info(outputfile, this->get_id());
  outputfile << "<tra:lod2MultiSurface>\n";
  outputfile << "<gml:MultiSurface>\n";
  for (auto& t : _triangles)
    get_triangle_as_gml_surfacemember(outputfile, t);
  for (auto& t : _triangles_vw)
    get_triangle_as_gml_surfacemember(outputfile, t, true);
  outputfile << "</gml:MultiSurface>\n";
  outputfile << "</tra:lod2MultiSurface>\n";
  std::string attribute;

  if (spoor) {
    if (get_attribute("bgt-functie", attribute)) {
      outputfile << "<tra:function codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FunctieSpoor\">" << attribute << "</tra:function>\n";
    }
    if (get_attribute("plus-functiespoor", attribute)) {
      outputfile << "<imgeo:plus-functieSpoor codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FunctieSpoorPlus\">" << attribute << "</imgeo:plus-functieSpoor>\n";
    }
    outputfile << "</tra:Railway>\n";
  }
  else if (auxiliary) {
    if (get_attribute("bgt-functie", attribute)) {
      outputfile << "<tra:function codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOndersteunendWegdeel\">" << attribute << "</tra:function>\n";
    }
    if (get_attribute("bgt-fysiekvoorkomen", attribute)) {
      outputfile << "<tra:surfaceMaterial codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenOndersteunendWegdeel\">" << attribute << "</imgeo:tra:surfaceMaterial>\n";
    }
    if (get_attribute("ondersteunendwegdeeloptalud", attribute, "false")) {
      outputfile << "<imgeo:ondersteunendWegdeelOpTalud>" << attribute << "</imgeo:ondersteunendWegdeelOpTalud>\n";
    }
    if (get_attribute("plus-functieondersteunendwegdeel", attribute)) {
      outputfile << "<imgeo:plus-functieOndersteunendWegdeel codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#TypeOndersteunendWegdeelPlus\">" << attribute << "</imgeo:plus-functieOndersteunendWegdeel>\n";
    }
    if (get_attribute("plus-fysiekvoorkomenondersteunendwegdeel", attribute)) {
      outputfile << "<imgeo:plus-fysiekVoorkomenOndersteunendWegdeel codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenOndersteunendWegdeelPlus\">" << attribute << "</imgeo:plus-fysiekVoorkomenOndersteunendWegdeel>\n";
    }
    outputfile << "</tra:AuxiliaryTrafficArea>\n";
  }
  else
  {
    if (get_attribute("bgt-functie", attribute)) {
      outputfile << "<tra:function codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FunctieWeg\">" << attribute << "</tra:function>\n";
    }
    if (get_attribute("bgt-fysiekvoorkomen", attribute)) {
      outputfile << "<tra:surfaceMaterial codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenWeg\">" << attribute << "</tra:surfaceMaterial>\n";
    }
    if (!get_attribute("wegdeeloptalud", attribute, "false")) {
      outputfile << "<imgeo:wegdeelOpTalud>" << attribute << "</imgeo:wegdeelOpTalud>\n";
    }
    if (get_attribute("plus-functiewegdeel", attribute)) {
      outputfile << "<imgeo:plus-functieWegdeel codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FunctieWegPlus\">" << attribute << "</imgeo:plus-functieWegdeel>\n";
    }
    if (get_attribute("plus-fysiekvoorkomenwegdeel", attribute)) {
      outputfile << "<imgeo:plus-fysiekVoorkomenWegdeel codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenWegPlus\">" << attribute << "</imgeo:plus-fysiekVoorkomenWegdeel>\n";
    }
    outputfile << "</tra:TrafficArea>\n";
  }
  outputfile << "</cityObjectMember>\n";
}

bool Road::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Road");
}
