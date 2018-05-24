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

#include "Building.h"
#include "io.h"
#include <algorithm>

//-- static variable
float Building::_heightref_top;
float Building::_heightref_base;
bool Building::_building_triangulate;
bool Building::_building_include_floor;
std::set<int> Building::_las_classes_roof;
std::set<int> Building::_las_classes_ground;
Building::Building(char *wkt, std::string layername, AttributeMap attributes, std::string pid, float heightref_top, float heightref_base, bool building_triangulate, bool building_include_floor)
  : Flat(wkt, layername, attributes, pid)
{
  _heightref_top = heightref_top;
  _heightref_base = heightref_base;
  _building_triangulate = building_triangulate;
  _building_include_floor = building_include_floor;
}

void Building::set_las_classes_roof(std::set<int> theset)
{
  Building::_las_classes_roof = theset;
}

void Building::set_las_classes_ground(std::set<int> theset)
{
  Building::_las_classes_ground = theset;
}

std::string Building::get_all_z_values() {
  std::vector<int> allz (_zvaluesground.begin(), _zvaluesground.end());
  allz.insert(allz.end(), _zvaluesinside.begin(), _zvaluesinside.end());
  std::sort(allz.begin(), allz.end());
  std::stringstream ss;
  for (auto& z : allz)
    ss << z << "|";
  return ss.str();
}

int Building::get_height_ground_at_percentile(float percentile) {
  if (_zvaluesground.empty() == false) {
    std::nth_element(_zvaluesground.begin(), _zvaluesground.begin() + (_zvaluesground.size() * percentile), _zvaluesground.end());
    return _zvaluesground[_zvaluesground.size() * percentile];
  }
  else {
    return -9999;
  }
}

int Building::get_height_roof_at_percentile(float percentile) {
  if (_zvaluesinside.empty() == false) {
    std::nth_element(_zvaluesinside.begin(), _zvaluesinside.begin() + (_zvaluesinside.size() * percentile), _zvaluesinside.end());
    return _zvaluesinside[_zvaluesinside.size() * percentile];
  }
  else {
    return -9999;
  }
}

bool Building::lift() {
  //-- for the ground
  if (_zvaluesground.empty() == false) {
    //-- Only use ground points for base height calculation
    std::nth_element(_zvaluesground.begin(), _zvaluesground.begin() + (_zvaluesground.size() * _heightref_base), _zvaluesground.end());
    _height_base = _zvaluesground[_zvaluesground.size() * _heightref_base];
  }
  else if (_zvaluesinside.empty() == false) {
    std::nth_element(_zvaluesinside.begin(), _zvaluesinside.begin() + (_zvaluesinside.size() * _heightref_base), _zvaluesinside.end());
    _height_base = _zvaluesinside[_zvaluesinside.size() * _heightref_base];
  }
  else {
    _height_base = -9999;
  }
  //-- for the roof
  Flat::lift_percentile(_heightref_top);
  return true;
}

bool Building::add_elevation_point(Point2 &p, double z, float radius, int lasclass) {
  if (within_range(p, *(_p2), radius)) {
    int zcm = int(z * 100);
    if ( (_las_classes_roof.empty() == true) || (_las_classes_roof.count(lasclass) > 0) ) {
      _zvaluesinside.push_back(zcm);
    }
    if ( (_las_classes_ground.empty() == true) || (_las_classes_ground.count(lasclass) > 0) ) {
      _zvaluesground.push_back(zcm);
    }
  }
  return true;
}

int Building::get_height_base() {
  return _height_base;
}

TopoClass Building::get_class() {
  return BUILDING;
}

bool Building::is_hard() {
  return true;
}

void Building::get_csv(std::wostream& of) {
  of << this->get_id() << ";" << std::setprecision(2) << std::fixed << this->get_height() << ";" << this->get_height_base() << "\n";
}

std::string Building::get_mtl() {
  return "usemtl Building";
}

void Building::get_obj(std::unordered_map< std::string, unsigned long > &dPts, int lod, std::string mtl, std::string &fs) {
  if (lod == 1) {
    TopoFeature::get_obj(dPts, mtl, fs);
  }
  else if (lod == 0) {
    fs += mtl;
    fs += "\n";
    for (auto& t : _triangles) {
      unsigned long a, b, c;
      float z = z_to_float(this->get_height_base());
      auto it = dPts.find(gen_key_bucket(&_vertices[t.v0].first, z));
      if (it == dPts.end()) {
        a = dPts.size() + 1;
        dPts[gen_key_bucket(&_vertices[t.v0].first, z)] = a;
      }
      else {
        a = it->second;
      }
      it = dPts.find(gen_key_bucket(&_vertices[t.v1].first, z));
      if (it == dPts.end()) {
        b = dPts.size() + 1;
        dPts[gen_key_bucket(&_vertices[t.v1].first, z)] = b;
      }
      else {
        b = it->second;
      }
      it = dPts.find(gen_key_bucket(&_vertices[t.v2].first, z));
      if (it == dPts.end()) {
        c = dPts.size() + 1;
        dPts[gen_key_bucket(&_vertices[t.v2].first, z)] = c;
      }
      else {
        c = it->second;
      }
      if ((a != b) && (a != c) && (b != c)) {
        fs += "f "; fs += std::to_string(a); fs += " "; fs += std::to_string(b); fs += " "; fs += std::to_string(c); fs += "\n";
      }
    }
  }
  if (_building_include_floor) {
    fs += "usemtl BuildingFloor\n";
    for (auto& t : _triangles) {
      unsigned long a, b, c;
      float z = z_to_float(this->get_height_base());
      auto it = dPts.find(gen_key_bucket(&_vertices[t.v0].first, z));
      if (it == dPts.end()) {
        a = dPts.size() + 1;
        dPts[gen_key_bucket(&_vertices[t.v0].first, z)] = a;
      }
      else {
        a = it->second;
      }
      it = dPts.find(gen_key_bucket(&_vertices[t.v1].first, z));
      if (it == dPts.end()) {
        b = dPts.size() + 1;
        dPts[gen_key_bucket(&_vertices[t.v1].first, z)] = b;
      }
      else {
        b = it->second;
      }
      it = dPts.find(gen_key_bucket(&_vertices[t.v2].first, z));
      if (it == dPts.end()) {
        c = dPts.size() + 1;
        dPts[gen_key_bucket(&_vertices[t.v2].first, z)] = c;
      }
      else {
        c = it->second;
      }
      //reverse orientation for floor polygon, a-c-b instead of a-b-c.
      if ((a != b) && (a != c) && (b != c)) {
        fs += "f "; fs += std::to_string(a); fs += " "; fs += std::to_string(c); fs += " "; fs += std::to_string(b); fs += "\n";
      }
    }
  }
}

void Building::get_cityjson(nlohmann::json& j, std::unordered_map<std::string, unsigned long> &dPts) {
  nlohmann::json b;
  b["type"] = "Building";
  b["attributes"];
  get_cityjson_attributes(b, _attributes);
  float hbase = z_to_float(this->get_height_base());
  float h = z_to_float(this->get_height());
  b["attributes"]["min-height-surface"] = hbase;
  b["attributes"]["measuredHeight"] = h;
  nlohmann::json g;
  this->get_cityjson_geom(g, dPts, "Solid");
  b["geometry"].push_back(g);
  j["CityObjects"][this->get_id()] = b;
}

void Building::get_citygml(std::wostream& of) {
  float h = z_to_float(this->get_height());
  float hbase = z_to_float(this->get_height_base());
  of << "<cityObjectMember>";
  of << "<bldg:Building gml:id=\"" << this->get_id() << "\">";
  get_citygml_attributes(of, _attributes);
  of << "<gen:measureAttribute name=\"min height surface\">";
  of << "<gen:value uom=\"#m\">" << hbase << "</gen:value>";
  of << "</gen:measureAttribute>";
  of << "<bldg:measuredHeight uom=\"#m\">" << h << "</bldg:measuredHeight>";
  //-- LOD0 footprint
  of << "<bldg:lod0FootPrint>";
  of << "<gml:MultiSurface>";
  get_polygon_lifted_gml(of, this->_p2, hbase, true);
  of << "</gml:MultiSurface>";
  of << "</bldg:lod0FootPrint>";
  //-- LOD0 roofedge
  of << "<bldg:lod0RoofEdge>";
  of << "<gml:MultiSurface>";
  get_polygon_lifted_gml(of, this->_p2, h, true);
  of << "</gml:MultiSurface>";
  of << "</bldg:lod0RoofEdge>";
  //-- LOD1 Solid
  of << "<bldg:lod1Solid>";
  of << "<gml:Solid>";
  of << "<gml:exterior>";
  of << "<gml:CompositeSurface>";
  if (_building_triangulate) {
    for (auto& t : _triangles)
      get_triangle_as_gml_surfacemember(of, t);
    for (auto& t : _triangles_vw)
      get_triangle_as_gml_surfacemember(of, t, true);
    if (_building_include_floor) {
      for (auto& t : _triangles) {
        get_floor_triangle_as_gml_surfacemember(of, t, _height_base);
      }
    }
  }
  else {
    get_extruded_lod1_block_gml(of, this->_p2, h, hbase, _building_include_floor);
  }
  of << "</gml:CompositeSurface>";
  of << "</gml:exterior>";
  of << "</gml:Solid>";
  of << "</bldg:lod1Solid>";
  of << "</bldg:Building>";
  of << "</cityObjectMember>";
}

void Building::get_citygml_imgeo(std::wostream& of) {
  float h = z_to_float(this->get_height());
  float hbase = z_to_float(this->get_height_base());
  of << "<cityObjectMember>";
  of << "<bui:Building gml:id=\"" << this->get_id() << "\">";
  //-- store building information
  get_imgeo_object_info(of, this->get_id());
  of << "<bui:consistsOfBuildingPart>";
  of << "<bui:BuildingPart>";
  //-- LOD1 Solid
  of << "<bui:lod1Solid>";
  of << "<gml:Solid>";
  of << "<gml:exterior>";
  of << "<gml:CompositeSurface>";
  if (_building_triangulate) {
    for (auto& t : _triangles)
      get_triangle_as_gml_surfacemember(of, t);
    for (auto& t : _triangles_vw)
      get_triangle_as_gml_surfacemember(of, t, true);
    if (_building_include_floor) {
      for (auto& t : _triangles) {
        get_floor_triangle_as_gml_surfacemember(of, t, _height_base);
      }
    }
  }
  else {
    if (_building_include_floor) {
      //-- get floor
      get_polygon_lifted_gml(of, this->_p2, hbase, false);
    }
    //-- get roof
    get_polygon_lifted_gml(of, this->_p2, h, true);
    //-- get the walls
    auto r = _p2->outer();
    int i;
    for (i = 0; i < (r.size() - 1); i++)
      get_extruded_line_gml(of, &r[i], &r[i + 1], h, hbase, false);
    get_extruded_line_gml(of, &r[i], &r[0], h, hbase, false);
    //-- irings
    auto irings = _p2->inners();
    for (Ring2& r : irings) {
      for (i = 0; i < (r.size() - 1); i++)
        get_extruded_line_gml(of, &r[i], &r[i + 1], h, hbase, false);
      get_extruded_line_gml(of, &r[i], &r[0], h, hbase, false);
    }
  }
  of << "</gml:CompositeSurface>";
  of << "</gml:exterior>";
  of << "</gml:Solid>";
  of << "</bui:lod1Solid>";
  std::string attribute;
  if (get_attribute("identificatiebagpnd", attribute)) {
    of << "<imgeo:identificatieBAGPND>" << attribute << "</imgeo:identificatieBAGPND>";
  }
  get_imgeo_nummeraanduiding(of);
  of << "</bui:BuildingPart>";
  of << "</bui:consistsOfBuildingPart>";
  of << "</bui:Building>";
  of << "</cityObjectMember>";
}

void Building::get_imgeo_nummeraanduiding(std::wostream& of) {
  std::string attribute;
  bool btekst, bplaatsingspunt, bhoek, blaagnr, bhoognr;
  std::string tekst, plaatsingspunt, hoek, laagnr, hoognr;
  btekst = get_attribute("tekst", tekst);
  bplaatsingspunt = get_attribute("plaatsingspunt", plaatsingspunt);
  bhoek = get_attribute("hoek", hoek);
  blaagnr = get_attribute("identificatiebagvbolaagstehuisnummer", laagnr);
  bhoognr = get_attribute("identificatiebagvbohoogstehuisnummer", hoognr);

  if (btekst) {
    // Split the lists into vector of strings
    std::vector<std::string> tekst_split, plaatsingspunt_split, hoek_split, laagnr_split, hoognr_split;
    tekst_split = stringsplit(tekst.substr(3, tekst.size() - 4), ',');
    if (bplaatsingspunt) {
      plaatsingspunt_split = stringsplit(plaatsingspunt.substr(3, plaatsingspunt.size() - 4), ',');
    }
    if (bhoek) {
      hoek_split = stringsplit(hoek.substr(3, hoek.size() - 4), ',');
    }
    if (blaagnr) {
      laagnr_split = stringsplit(laagnr.substr(3, laagnr.size() - 4), ',');
    }
    if (bhoognr) {
      hoognr_split = stringsplit(hoognr.substr(3, hoognr.size() - 4), ',');
    }

    //Get the amount of text in the StringList and write all List values separate
    int count = boost::lexical_cast<int>(tekst[1]);
    for (int i = 0; i < count; i++) {
      if (i < tekst_split.size() && i < plaatsingspunt_split.size() && i < hoek_split.size()) {
        of << "<imgeo:nummeraanduidingreeks>";
        of << "<imgeo:Nummeraanduidingreeks>";
        of << "<imgeo:nummeraanduidingreeks>";
        of << "<imgeo:Label>";
        of << "<imgeo:tekst>" << tekst_split.at(i) << "</imgeo:tekst>";
        of << "<imgeo:positie>";
        of << "<imgeo:Labelpositie>";
        of << "<imgeo:plaatsingspunt><gml:Point srsDimension=\"2\"><gml:pos>" << plaatsingspunt_split.at(i) << "</gml:pos></gml:Point></imgeo:plaatsingspunt>";
        of << "<imgeo:hoek>" << hoek_split.at(i) << "</imgeo:hoek>";
        of << "</imgeo:Labelpositie>";
        of << "</imgeo:positie>";
        of << "</imgeo:Label>";
        of << "</imgeo:nummeraanduidingreeks>";
        if (i < laagnr_split.size()) {
          of << "<imgeo:identificatieBAGVBOLaagsteHuisnummer>" << laagnr_split.at(i) << "</imgeo:identificatieBAGVBOLaagsteHuisnummer>";
        }
        if (i < hoognr_split.size()) {
          of << "<imgeo:identificatieBAGVBOHoogsteHuisnummer>" << hoognr_split.at(i) << "</imgeo:identificatieBAGVBOHoogsteHuisnummer>";
        }
        of << "</imgeo:Nummeraanduidingreeks>";
        of << "</imgeo:nummeraanduidingreeks>";
      }
    }
  }
}

bool Building::get_shape(OGRLayer* layer, bool writeAttributes, AttributeMap extraAttributes) {
  return TopoFeature::get_multipolygon_features(layer, "Building", writeAttributes, extraAttributes, true, this->get_height_base(), this->get_height());
}
