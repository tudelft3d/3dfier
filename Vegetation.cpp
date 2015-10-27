//
//  Vegetation.cpp
//
//  Created by Hugo Ledoux on 19/10/15.
//  Copyright © 2015 Hugo Ledoux. All rights reserved.
//

#include "Vegetation.h"


Vegetation::Vegetation (Polygon2* p, std::string pid, int simplification) : TIN(p, pid, simplification)
{}


bool Vegetation::threeDfy() {
  std::stringstream ss;
  ss << bg::wkt(*(_p2));
  Polygon3 p3;
  bg::read_wkt(ss.str(), p3);
  TopoFeature::lift_boundary(p3, 0.5);
  getCDT(&p3, _vertices, _triangles, _segments, _lidarpts);
  return true;
}

std::string Vegetation::get_citygml() {
  return "<EMPTY/>";
}

std::string Vegetation::get_obj_f(int offset) {
  std::stringstream ss;
  ss << "usemtl Vegetation" << std::endl;
  ss << TopoFeature::get_obj_f(offset);
  return ss.str();
}


