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

#include "Bridge.h"

float Bridge::_heightref = 0.8;

Bridge::Bridge(char *wkt, std::string pid, float heightref)
  : Flat(wkt, pid)
{
  _heightref = heightref;
}


bool Bridge::lift() {
  //lift_each_boundary_vertices(percentile);
  //smooth_boundary(5);
  lift_percentile(_heightref);
  return true;
}


bool Bridge::add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  if (lastreturn == true && lasclass != LAS_BUILDING && lasclass != LAS_WATER) {
    Flat::add_elevation_point(x, y, z, radius, lasclass, lastreturn);
  }
  return true;
}


TopoClass Bridge::get_class() {
  return BRIDGE;
}


bool Bridge::is_hard() {
  return true;
}


std::string Bridge::get_mtl() {
  return "usemtl Bridge\n";
}


std::string Bridge::get_citygml() {
  return "<EMPTY/>";
}


bool Bridge::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Bridge");
}
