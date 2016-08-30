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

float Road::_heightref = 0.5;

Road::Road (char *wkt, std::string pid, float heightref)
: Boundary3D(wkt, pid)
{
    _heightref = heightref;
}


bool Road::lift() {
  lift_each_boundary_vertices(_heightref);
  smooth_boundary(5);
  return true;
}


bool Road::add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  if (lastreturn == true && lasclass == LAS_GROUND)
    assign_elevation_to_vertex(x, y, z, radius);
  return true;
}


TopoClass Road::get_class() {
  return ROAD;
}


bool Road::is_hard() {
  return true;
}


std::string Road::get_mtl() {
  return "usemtl Road\n";
}


std::string Road::get_citygml() {
  return "<EMPTY/>";
}


bool Road::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Road");
}
