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
: Flat(wkt, pid)
{
  _heightref = heightref;
}

bool Water::lift() {
  double percentile = (std::stod(_heightref.substr(_heightref.find_first_of("-") + 1)) / 100);
  Flat::lift_percentile(percentile);
  return true;
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


