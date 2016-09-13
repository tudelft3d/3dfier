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

float Building::_heightref_top  = 0.9;
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
  else if(_zvaluesinside.empty() == false) {
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


bool Building::add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  if (lastreturn) {
    int zcm = int(z * 100);
    //-- 1. Save the ground points seperate for base height
    if (lasclass == LAS_GROUND || lasclass == LAS_WATER) {
      _zvaluesground.push_back(zcm);
    }
    Point2 p(x, y);
    if (bg::distance(p, *(_p2)) <= radius) {
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

std::string Building::get_obj_v_building_volume(std::vector<Point3>::size_type &idx, std::unordered_map<std::string, std::vector<Point3>::size_type> &vertices_map, int z_exaggeration) {
  std::stringstream ss;
  for (auto& v : _vertices) {
    std::string key = gen_key_bucket(&v);
    if (vertices_map.find(key) == vertices_map.end()) {
      vertices_map[key] = idx;
      idx++;
      ss << std::setprecision(3) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << (z_exaggeration > 0 ? (z_exaggeration * bg::get<2>(v)) : bg::get<2>(v)) << std::endl;
    }
  }
  for (auto& v : _vertices) {
    std::string key = gen_key_bucket(&v);
    if (vertices_map.find(key) == vertices_map.end()) {
      vertices_map[key] = idx;
      idx++;
      float z = float(this->get_height_base()) / 100;
      ss << std::setprecision(3) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << (z_exaggeration > 0 ? (z_exaggeration * z) : z) << std::endl;
    }
  }
  return ss.str();
}


std::string Building::get_obj_f_building_volume(std::unordered_map<std::string, std::vector<Point3>::size_type> &vertices_map, bool usemtl) {
  std::stringstream ss;
  if (usemtl == true)
    ss << "usemtl Building" << std::endl;
//-- top surface
  for (auto& t : _triangles)
    ss << "f " << _vertices_map[t.v0] << " " << _vertices_map[t.v1] << " " << _vertices_map[t.v2] << std::endl;
//-- ground surface
  for (auto& t : _triangles)
    ss << "f " << _vertices_map[t.v0] + _vertices.size() << " " << _vertices_map[t.v1] + _vertices.size() << " " << _vertices_map[t.v2] + _vertices.size() << std::endl;
//-- extract segments
  std::vector<Segment> allsegments;
  for (auto& curt : _triangles) {
    bool issegment = false;
    for (auto& t : _triangles) {
      if (triangle_contains_segment(t, curt.v1, curt.v0) == true) {
        issegment = true;
        break;
      }
    }
    if (issegment == false) {
      Segment s;
      s.v0 = curt.v0;
      s.v1 = curt.v1;
      allsegments.push_back(s);
    }
    issegment = false;
    for (auto& t : _triangles) {
      if (triangle_contains_segment(t, curt.v2, curt.v1) == true) {
        issegment = true;
        break;
      }
    }
    if (issegment == false) {
      Segment s;
      s.v0 = curt.v1;
      s.v1 = curt.v2;
      allsegments.push_back(s);
    }
    issegment = false;
    for (auto& t : _triangles) {
      if (triangle_contains_segment(t, curt.v0, curt.v2) == true) {
        issegment = true;
        break;
      }
    }
    if (issegment == false) {
      Segment s;
      s.v0 = curt.v2;
      s.v1 = curt.v0;
      allsegments.push_back(s);
    }
  }
  //-- side surfaces walls
  for (auto& s : allsegments) {
    //ss << "f " << (s.v1 + 1 + offset) << " " << (s.v0 + 1 + offset) << " " << (s.v0 + 1 + offset + _vertices.size()) << std::endl;  
    //ss << "f " << (s.v0 + 1 + offset + _vertices.size()) << " " << (s.v1 + 1 + offset + _vertices.size()) << " " << (s.v1 + 1 + offset) << std::endl;  
    ss << "f " << _vertices_map[s.v1] << " " << _vertices_map[s.v0]  << " " << _vertices_map[s.v0] + _vertices.size() << std::endl;
    ss << "f " << _vertices_map[s.v0] + _vertices.size() << " " << _vertices_map[s.v1] + _vertices.size() << " " << _vertices_map[s.v1] << std::endl;
 }
  return ss.str();
}

std::string Building::get_obj_f_floor(std::unordered_map<std::string, std::vector<Point3>::size_type> &vertices_map) {
  std::stringstream ss;
  for (auto& t : _triangles) {
    //ss << "f " << (t.v0 + 1 + offset + _vertices.size()) << " " << (t.v2 + 1 + offset + _vertices.size()) << " " << (t.v1 + 1 + offset + _vertices.size()) << std::endl;
    ss << "f " << _vertices_map[t.v0] + _vertices.size() << " " << _vertices_map[t.v2] + _vertices.size() << " " << _vertices_map[t.v1] + _vertices.size() << std::endl;
  }
  return ss.str();
}

std::string Building::get_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>";
  ss << "<bldg:Building>";
  ss << "<bldg:measuredHeight uom=\"#m\">";
  ss << this->get_height();
  ss << "</bldg:measuredHeight>";
  ss << "<bldg:lod1Solid>";
  ss << "<gml:Solid>";
  ss << "<gml:exterior>";
  ss << "<gml:CompositeSurface>";
  //-- get floor
  ss << get_polygon_lifted_gml(this->_p2, this->get_height_base(), false);
  //-- get roof
  ss << get_polygon_lifted_gml(this->_p2, this->get_height(), true);
  //-- get the walls
  auto r = bg::exterior_ring(*(this->_p2));
  for (int i = 0; i < (r.size() - 1); i++) 
    ss << get_extruded_line_gml(&r[i], &r[i + 1], this->get_height(), 0, false);
  ss << "</gml:CompositeSurface>";
  ss << "</gml:exterior>";
  ss << "</gml:Solid>";
  ss << "</bldg:lod1Solid>";
  ss << "</bldg:Building>";
  ss << "</cityObjectMember>";
  return ss.str(); 
}


bool Building::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Building");
}