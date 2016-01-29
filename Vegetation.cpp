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

 
#include "Vegetation.h"


Vegetation::Vegetation (char *wkt, std::string pid, int simplification) : TIN(wkt, pid, simplification)
{}


bool Vegetation::lift() {
  TopoFeature::lift_vertices_boundary(0.5);
  return true;
}

bool Vegetation::buildCDT() {
  getCDT(&_p3, _vertices, _triangles, _segments, _lidarpts);
  construct_vertical_walls();
  return true;
}

TopoClass Vegetation::get_class() {
  return VEGETATION;
}

bool Vegetation::is_hard() {
  return false;
}

std::string Vegetation::get_citygml() {
  return "<EMPTY/>";
}

std::string Vegetation::get_obj_f(int offset, bool usemtl) {
  std::stringstream ss;
  if (usemtl == true)
    ss << "usemtl Vegetation" << std::endl;
  ss << TopoFeature::get_obj_f(offset, usemtl);
  return ss.str();
}


