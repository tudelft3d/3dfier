//
//  Water.cpp
//
//  Created by Hugo Ledoux on 19/10/15.
//  Copyright © 2015 Hugo Ledoux. All rights reserved.
//

#include "Water.h"

std::string Water::_heightref = "percentile-10";

Water::Water (Polygon2* p, std::string pid, std::string heightref) : Boundary3D(p, pid)
{
  _heightref = heightref;
}

bool Water::threeDfy() {
  std::stringstream ss;
  ss << bg::wkt(*(_p2));
  Polygon3 p3;
  bg::read_wkt(ss.str(), p3);
  lift_boundary(p3, 0.1);
  getCDT(&p3, _vertices, _triangles, _segments);
  _velevations.clear();
  _velevations.shrink_to_fit();
  return true;
}

std::string Water::get_obj_f(int offset) {
  std::stringstream ss;
  ss << "usemtl Water" << std::endl;
  ss << TopoFeature::get_obj_f(offset);
  return ss.str();
}

std::string Water::get_citygml() {
  return "<EMPTY/>";
}


