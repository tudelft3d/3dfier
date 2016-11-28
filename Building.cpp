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

#include "Building.h"
#include "io.h"

float Building::_heightref_top = 0.9;
float Building::_heightref_base = 0.1;

Building::Building(char *wkt, std::string pid, float heightref_top, float heightref_base)
  : Flat(wkt, pid)
{
  _heightref_top = heightref_top;
  _heightref_base = heightref_base;
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
    _height_base = 0;
  }

  //-- for the roof
  Flat::lift_percentile(_heightref_top);

  return true;
}

bool Building::add_elevation_point(Point2 p, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  if (lastreturn) {
    if (bg::distance(p, *(_p2)) <= radius) {
      int zcm = int(z * 100);
      //-- 1. Save the ground points seperate for base height
      if (lasclass == LAS_GROUND || lasclass == LAS_WATER) {
        _zvaluesground.push_back(zcm);
      }
      //-- 2. assign to polygon since within
      _zvaluesinside.push_back(zcm);
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

std::string Building::get_csv() {
  std::stringstream ss;
  ss << this->get_id() << ";" << std::setprecision(2) << std::fixed << this->get_height() << ";" << this->get_height_base() << std::endl;
  return ss.str();
}

std::string Building::get_mtl() {
  return "usemtl Building\n";
}

std::string Building::get_obj(std::unordered_map< std::string, unsigned long > &dPts, int lod) {
  std::stringstream ss;
  if (lod == 1) {
    ss << TopoFeature::get_obj(dPts);
  }
  else if (lod == 0) {
    for (auto& t : _triangles) {
      unsigned long a, b, c;
      int z = this->get_height_base();
      auto it = dPts.find(gen_key_bucket(&_vertices[t.v0], z));
      if (it == dPts.end()) {
        a = dPts.size() + 1;
        dPts[gen_key_bucket(&_vertices[t.v0], z)] = a;
      }
      else {
        a = it->second;
      }
      it = dPts.find(gen_key_bucket(&_vertices[t.v1], z));
      if (it == dPts.end()) {
        b = dPts.size() + 1;
        dPts[gen_key_bucket(&_vertices[t.v1], z)] = b;
      }
      else {
        b = it->second;
      }
      it = dPts.find(gen_key_bucket(&_vertices[t.v2], z));
      if (it == dPts.end()) {
        c = dPts.size() + 1;
        dPts[gen_key_bucket(&_vertices[t.v2], z)] = c;
      }
      else {
        c = it->second;
      }
      if ((a != b) && (a != c) && (b != c))
        ss << "f " << a << " " << b << " " << c << std::endl;
      // else
      //   std::clog << "COLLAPSED TRIANGLE REMOVED" << std::endl;
    }
  }
  return ss.str();
}

std::string Building::get_citygml() {
  float h = z_to_float(this->get_height());
  float hbase = z_to_float(this->get_height_base());
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<bldg:Building gml:id=\"";
  ss << this->get_id();
  ss << "\">" << std::endl;
  ss << "<bldg:measuredHeight uom=\"#m\">";
  ss << h;
  ss << "</bldg:measuredHeight>" << std::endl;
  //-- LOD0 footprint
  ss << "<bldg:lod0FootPrint>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << get_polygon_lifted_gml(this->_p2, hbase, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</bldg:lod0FootPrint>" << std::endl;
  //-- LOD0 roofedge
  ss << "<bldg:lod0RoofEdge>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << get_polygon_lifted_gml(this->_p2, h, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</bldg:lod0RoofEdge>" << std::endl;
  //-- LOD1 Solid
  ss << "<bldg:lod1Solid>" << std::endl;
  ss << "<gml:Solid>" << std::endl;
  ss << "<gml:exterior>" << std::endl;
  ss << "<gml:CompositeSurface>" << std::endl;
  //-- get floor
  ss << get_polygon_lifted_gml(this->_p2, hbase, false);
  //-- get roof
  ss << get_polygon_lifted_gml(this->_p2, h, true);
  //-- get the walls
  auto r = bg::exterior_ring(*(this->_p2));
  int i;
  for (i = 0; i < (r.size() - 1); i++)
    ss << get_extruded_line_gml(&r[i], &r[i + 1], h, hbase, false);
  ss << get_extruded_line_gml(&r[i], &r[0], h, hbase, false);
  //-- irings
  auto irings = bg::interior_rings(*(this->_p2));
  for (Ring2& r : irings) {
    for (i = 0; i < (r.size() - 1); i++)
      ss << get_extruded_line_gml(&r[i], &r[i + 1], h, hbase, false);
    ss << get_extruded_line_gml(&r[i], &r[0], h, hbase, false);
  }
  ss << "</gml:CompositeSurface>" << std::endl;
  ss << "</gml:exterior>" << std::endl;
  ss << "</gml:Solid>" << std::endl;
  ss << "</bldg:lod1Solid>" << std::endl;
  ss << "</bldg:Building>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}


std::string Building::get_citygml_imgeo() {
  float h = z_to_float(this->get_height());
  float hbase = z_to_float(this->get_height_base());
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<bui:BuildingPart gml:id=\"" << this->get_id() << "\">" << std::endl;
  ss << get_imgeo_object_info(this->get_id());
  //-- LOD1 Solid
  ss << "<bui:lod1Solid>" << std::endl;
  ss << "<gml:Solid>" << std::endl;
  ss << "<gml:exterior>" << std::endl;
  ss << "<gml:CompositeSurface>" << std::endl;
  //-- get floor
  ss << get_polygon_lifted_gml(this->_p2, hbase, false);
  //-- get roof
  ss << get_polygon_lifted_gml(this->_p2, h, true);
  //-- get the walls
  auto r = bg::exterior_ring(*(this->_p2));
  int i;
  for (i = 0; i < (r.size() - 1); i++)
    ss << get_extruded_line_gml(&r[i], &r[i + 1], h, hbase, false);
  ss << get_extruded_line_gml(&r[i], &r[0], h, hbase, false);
  //-- irings
  auto irings = bg::interior_rings(*(this->_p2));
  for (Ring2& r : irings) {
    for (i = 0; i < (r.size() - 1); i++)
      ss << get_extruded_line_gml(&r[i], &r[i + 1], h, hbase, false);
    ss << get_extruded_line_gml(&r[i], &r[0], h, hbase, false);
  }
  ss << "</gml:CompositeSurface>" << std::endl;
  ss << "</gml:exterior>" << std::endl;
  ss << "</gml:Solid>" << std::endl;
  ss << "</bui:lod1Solid>" << std::endl;
  ss << "<imgeo:identificatieBAGPND>" << /*bagpnd*/ "0" << "</imgeo:identificatieBAGPND>" << std::endl;
  ss << "<imgeo:nummeraanduidingreeks>" << std::endl;
  ss << "<imgeo:Nummeraanduidingreeks>" << std::endl;
  ss << "<imgeo:nummeraanduidingreeks>" << std::endl;
  ss << "<imgeo:Label>" << std::endl;
  ss << "<imgeo:tekst>" << "Tekst" << "</imgeo:tekst>" << std::endl;
  ss << "<imgeo:positie>" << std::endl;
  ss << "<imgeo:Labelpositie>" << std::endl;
  ss << "<imgeo:plaatsingspunt>" << std::endl;
  ss << "<gml:Point srsDimension=\"2\">" << std::endl;
  ss << "<gml:pos>" << /*x-posistion*/ "0" << " " << /*y-position*/ "0" << "</gml:pos>" << std::endl;
  ss << "</gml:Point>" << std::endl;
  ss << "</imgeo:plaatsingspunt>" << std::endl;
  ss << "<imgeo:hoek>" << /*hoek*/ "0" << "</imgeo:hoek>" << std::endl;
  ss << "</imgeo:Labelpositie>" << std::endl;
  ss << "</imgeo:positie>" << std::endl;
  ss << "</imgeo:Label>" << std::endl;
  ss << "</imgeo:nummeraanduidingreeks>" << std::endl;
  ss << "<imgeo:identificatieBAGVBOLaagsteHuisnummer>" << /*laagsteHuisnummer*/ "identificatieBAGVBOLaagsteHuisnummer0" << "</imgeo:identificatieBAGVBOLaagsteHuisnummer>" << std::endl;
  ss << "<imgeo:identificatieBAGVBOHoogsteHuisnummer>" << /*hoogsteHuisnummer*/ "identificatieBAGVBOHoogsteHuisnummer0" << "</imgeo:identificatieBAGVBOHoogsteHuisnummer>" << std::endl;
  ss << "</imgeo:Nummeraanduidingreeks>" << std::endl;
  ss << "</imgeo:nummeraanduidingreeks>" << std::endl;
  ss << "</bui:BuildingPart>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}


bool Building::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Building");
}
