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
 
#include "Road.h"

std::string Road::_heightref = "percentile-10";

Road::Road (char *wkt, std::string pid, std::string heightref) 
: Boundary3D(wkt, pid)
{
  if (heightref != "")
    _heightref = heightref;
}

bool Road::lift() {
  //-- assign an elevation to each vertex
  float percentile = std::stof(_heightref.substr(_heightref.find_first_of("-") + 1)) / 100;
  lift_vertices_boundary(percentile);
  //-- take minimum value for obtaining horizontal value
  smooth_boundary(5);
  return true;
}


bool Road::buildCDT() {
  getCDT(&_p3, _vertices, _triangles, _segments);
  return true;
}


TopoClass Road::get_class() {
  return ROAD;
}

bool Road::is_hard() {
  return true;
}


std::string Road::get_obj_f(int offset) {
  std::stringstream ss;
  ss << "usemtl Road" << std::endl;
  ss << TopoFeature::get_obj_f(offset);
  return ss.str();
}

std::string Road::get_citygml() {
  return "<EMPTY/>";
}


