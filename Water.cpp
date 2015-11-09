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
  if (heightref != "")
    _heightref = heightref;
}

bool Water::lift() {
  std::stringstream ss;
  ss << bg::wkt(*(_p2));
  bg::read_wkt(ss.str(), _p3);
  //-- assign an elevation to each vertex
  float percentile = std::stof(_heightref.substr(_heightref.find_first_of("-") + 1)) / 100;
  lift_vertices_boundary(percentile);
  //-- take minimum value for obtaining horizontal value
  this->make_boundary_horizontal();
  return true;
}

bool Water::buildCDT() {
  getCDT(&_p3, _vertices, _triangles, _segments);
  return true;
}

TopoClass Water::get_class() {
  return WATER;
}

void Water::make_boundary_horizontal() {
  //-- find the mininum value and assign it
  double minimum = 1e9;
  Ring3 oring = bg::exterior_ring(_p3);
  for (int i = 0; i < oring.size(); i++) 
    if (bg::get<2>(bg::exterior_ring(_p3)[i]) < minimum)
      minimum = bg::get<2>(bg::exterior_ring(_p3)[i]);
  auto irings = bg::interior_rings(_p3);
  std::size_t offset = bg::num_points(oring);
  for (int i = 0; i < irings.size(); i++) {
    for (int j = 0; j < (irings[i]).size(); j++)
      if (bg::get<2>(bg::interior_rings(_p3)[i][j]) < minimum)
        minimum = bg::get<2>(bg::interior_rings(_p3)[i][j]);
    offset += bg::num_points(irings[i]);
  }
  //-- assign minimum value
  oring = bg::exterior_ring(_p3);
  for (int i = 0; i < oring.size(); i++) 
    bg::set<2>(bg::exterior_ring(_p3)[i], minimum);
  irings = bg::interior_rings(_p3);
  offset = bg::num_points(oring);
  for (int i = 0; i < irings.size(); i++) {
    for (int j = 0; j < (irings[i]).size(); j++)
      bg::set<2>(bg::interior_rings(_p3)[i][j], minimum);
    offset += bg::num_points(irings[i]);
  }
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


