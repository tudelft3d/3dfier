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

 
#include "Water.h"

std::string Water::_heightref = "percentile-10";

Water::Water (char *wkt, std::string pid, std::string heightref) 
: TopoFeature(wkt, pid)
{
  if (heightref != "")
    _heightref = heightref;
}

bool Water::lift() {
  float z = 0.0;
  if (_zvaluesinside.empty() == false) {
    float percentile = std::stof(_heightref.substr(_heightref.find_first_of("-") + 1)) / 100;
    std::nth_element(_zvaluesinside.begin(), _zvaluesinside.begin() + (_zvaluesinside.size() * percentile), _zvaluesinside.end());
    //-- assign an elevation to each vertex
    z = _zvaluesinside[_zvaluesinside.size() * percentile];
  }
  //-- assign minimum value
  Ring3 oring = bg::exterior_ring(_p3);
  for (int i = 0; i < oring.size(); i++) 
    bg::set<2>(bg::exterior_ring(_p3)[i], z);
  auto irings = bg::interior_rings(_p3);
  std::size_t offset = bg::num_points(oring);
  for (int i = 0; i < irings.size(); i++) {
    for (int j = 0; j < (irings[i]).size(); j++)
      bg::set<2>(bg::interior_rings(_p3)[i][j], z);
    offset += bg::num_points(irings[i]);
  }
  return true;
}

bool Water::add_elevation_point(double x, double y, double z, float radius)
{
  Point2 p(x, y);
  //-- 1. assign to polygon if inside
  if (bg::within(p, *(_p2)) == true)
    _zvaluesinside.push_back(z);
  return true;
}

bool Water::buildCDT() {
  getCDT(&_p3, _vertices, _triangles, _segments);
  return true;
}


int Water::get_number_vertices() {
  return int(_vertices.size());
}

TopoClass Water::get_class() {
  return WATER;
}

bool Water::is_hard() {
  return true;
}

std::string Water::get_obj_f(int offset, bool usemtl) {
  std::stringstream ss;
  if (usemtl == true)
    ss << "usemtl Water" << std::endl;
  ss << TopoFeature::get_obj_f(offset, usemtl);
  return ss.str();
}

std::string Water::get_citygml() {
  return "<EMPTY/>";
}


