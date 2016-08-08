/*
 Copyright (c) 2015 Hugo Ledoux
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#include "Building.h"
#include "io.h"

std::string Building::_heightref_top  = "percentile-90";
std::string Building::_heightref_base = "percentile-10";

Building::Building(char *wkt, std::string pid, std::string heightref_top, std::string heightref_base)
: Flat(wkt, pid)
{
  _heightref_top = heightref_top;
  _heightref_base = heightref_base;
}

bool Building::lift() {
  //-- for the ground
  double percentile = (std::stod(_heightref_base.substr(_heightref_base.find_first_of("-") + 1)) / 100);

  if (_zvaluesground.empty() == false) {
    //-- Only use ground points for base height calculation
    std::nth_element(_zvaluesground.begin(), _zvaluesground.begin() + (_zvaluesground.size() * percentile), _zvaluesground.end());
    _height_base = _zvaluesground[_zvaluesground.size() * percentile];
  }
  else if(_zvaluesinside.empty() == false) {
    std::nth_element(_zvaluesinside.begin(), _zvaluesinside.begin() + (_zvaluesinside.size() * percentile), _zvaluesinside.end());
    _height_base = _zvaluesinside[_zvaluesinside.size() * percentile];
  }
  else {
    _height_base = 0;
  }

  //-- for the roof
  percentile = (std::stod(_heightref_top.substr(_heightref_top.find_first_of("-") + 1)) / 100);
  Flat::lift_percentile(percentile);

  return true;
}


bool Building::add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  if (lastreturn) {
    int zcm = int(z * 100);
    //-- 1. Save the ground points seperate for base height
    if (lasclass == LAS_GROUND || lasclass == LAS_WATER) {
      _zvaluesground.push_back(zcm);
    }
    //-- 2. assign to polygon since within the threshold value (buffering of polygon)
    _zvaluesinside.push_back(zcm);
    //-- 3. add to the vertices of the pgn to find their heights
    assign_elevation_to_vertex(x, y, z, radius);
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