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
  //-- for the roof
  double percentile = (std::stod(_heightref_top.substr(_heightref_top.find_first_of("-") + 1)) / 100);
  Flat::lift_percentile(percentile);
  //-- for the ground
  percentile = (std::stod(_heightref_base.substr(_heightref_base.find_first_of("-") + 1)) / 100);
  std::vector<int> tmp;
  int ringi = 0;
  Ring2 oring = bg::exterior_ring(*(_p2));
  for (int i = 0; i < oring.size(); i++) {
    std::vector<int> &l = _lidarelevs[ringi][i];
    std::nth_element(l.begin(), l.begin() + (l.size() * percentile), l.end());
    if (l.empty() == false)
      tmp.push_back(l[l.size() * percentile]);
  }
  ringi++;
  auto irings = bg::interior_rings(*(_p2));
  for (Ring2& iring: irings) {
    for (int i = 0; i < iring.size(); i++) {
      std::vector<int> &l = _lidarelevs[ringi][i];
      std::nth_element(l.begin(), l.begin() + (l.size() * percentile), l.end());
      if (l.empty() == false)
        tmp.push_back(l[l.size() * percentile]);
    }
    ringi++;
  }
  if (tmp.empty())
    _height_base = 0;
  else //-- take the lowest of all the vertices for the ground
    _height_base = *(std::min_element(std::begin(tmp), std::end(tmp)));
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

std::string Building::get_obj_f(int offset, bool usemtl) {
  std::stringstream ss;
  if (usemtl == true)
    ss << "usemtl Building" << std::endl;
  ss << TopoFeature::get_obj_f(offset, usemtl);
  return ss.str();
}

std::string Building::get_obj_v_building_volume(int z_exaggeration) {
  std::stringstream ss;
  for (auto& v : _vertices)
    ss << std::setprecision(3) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << (z_exaggeration > 0? (z_exaggeration * bg::get<2>(v)) : bg::get<2>(v)) << std::endl;
  for (auto& v : _vertices) {
    float z = float(this->get_height_base()) / 100;
    ss << std::setprecision(3) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << (z_exaggeration > 0? (z_exaggeration * z) : z) << std::endl;
  }
  return ss.str();
}


std::string Building::get_obj_f_building_volume(int offset, bool usemtl) {
  std::stringstream ss;
  if (usemtl == true)
    ss << "usemtl Building" << std::endl;
//-- top surface
  for (auto& t : _triangles)
    ss << "f " << (t.v0 + 1 + offset) << " " << (t.v1 + 1 + offset) << " " << (t.v2 + 1 + offset) << std::endl;
//-- ground surface
  for (auto& t : _triangles)
    ss << "f " << (t.v0 + 1 + offset + _vertices.size()) << " " << (t.v2 + 1 + offset + _vertices.size()) << " " << (t.v1 + 1 + offset + _vertices.size()) << std::endl;  
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
    ss << "f " << (s.v1 + 1 + offset) << " " << (s.v0 + 1 + offset) << " " << (s.v0 + 1 + offset + _vertices.size()) << std::endl;  
    ss << "f " << (s.v0 + 1 + offset + _vertices.size()) << " " << (s.v1 + 1 + offset + _vertices.size()) << " " << (s.v1 + 1 + offset) << std::endl;  
  }
  return ss.str();
}

std::string Building::get_obj_f_floor(int offset) {
  std::stringstream ss;
  for (auto& t : _triangles)
    ss << "f " << (t.v0 + 1 + offset + _vertices.size()) << " " << (t.v2 + 1 + offset + _vertices.size()) << " " << (t.v1 + 1 + offset + _vertices.size()) << std::endl;  
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