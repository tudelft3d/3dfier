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

bool Water::threeDfy() {
  std::stringstream ss;
  ss << bg::wkt(*(_p2));
  Polygon3 p3;
  bg::read_wkt(ss.str(), p3);
  //-- assign an elevation to each vertex
  float percentile = std::stof(_heightref.substr(_heightref.find_first_of("-") + 1)) / 100;
  lift_vertices_boundary(p3, percentile);
  //-- take minimum value for obtaining horizontal value
  make_boundary_horizontal(p3);
  //-- triangulate
  getCDT(&p3, _vertices, _triangles, _segments);
  return true;
}


void Water::make_boundary_horizontal(Polygon3 &p3) {
  //-- find the mininum value and assign it
  double minimum = 1e9;
  Ring3 oring = bg::exterior_ring(p3);
  for (int i = 0; i < oring.size(); i++) 
    if (bg::get<2>(bg::exterior_ring(p3)[i]) < minimum)
      minimum = bg::get<2>(bg::exterior_ring(p3)[i]);
  auto irings = bg::interior_rings(p3);
  std::size_t offset = bg::num_points(oring);
  for (int i = 0; i < irings.size(); i++) {
    for (int j = 0; j < (irings[i]).size(); j++)
      if (bg::get<2>(bg::interior_rings(p3)[i][j]) < minimum)
        minimum = bg::get<2>(bg::interior_rings(p3)[i][j]);
    offset += bg::num_points(irings[i]);
  }
  //-- assign minimum value
  oring = bg::exterior_ring(p3);
  for (int i = 0; i < oring.size(); i++) 
    bg::set<2>(bg::exterior_ring(p3)[i], minimum);
  irings = bg::interior_rings(p3);
  offset = bg::num_points(oring);
  for (int i = 0; i < irings.size(); i++) {
    for (int j = 0; j < (irings[i]).size(); j++)
      bg::set<2>(bg::interior_rings(p3)[i][j], minimum);
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


