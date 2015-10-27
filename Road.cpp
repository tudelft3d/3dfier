//
//  Road.cpp
//
//  Created by Hugo Ledoux on 19/10/15.
//  Copyright © 2015 Hugo Ledoux. All rights reserved.
//

#include "Road.h"

std::string Road::_heightref = "percentile-10";

Road::Road (Polygon2* p, std::string pid, std::string heightref) : Boundary3D(p, pid)
{
  if (heightref != "")
    _heightref = heightref;
}

bool Road::threeDfy() {
  std::stringstream ss;
  ss << bg::wkt(*(_p2));
  Polygon3 p3;
  bg::read_wkt(ss.str(), p3);
  //-- assign an elevation to each vertex
  float percentile = std::stof(_heightref.substr(_heightref.find_first_of("-") + 1)) / 100;
  lift_vertices_boundary(p3, percentile);
  //-- take minimum value for obtaining horizontal value
  smooth_boundary(p3, 5);
  //-- triangulate
  getCDT(&p3, _vertices, _triangles, _segments);
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


