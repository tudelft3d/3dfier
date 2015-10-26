//
//  Water.cpp
//
//  Created by Hugo Ledoux on 19/10/15.
//  Copyright © 2015 Hugo Ledoux. All rights reserved.
//

#include "Water.h"


Water::Water (Polygon2* p, std::string pid) : Boundary3D(p, pid)
{}


std::string Water::get_obj_f(int offset) {
  std::stringstream ss;
  ss << "usemtl Water" << std::endl;
  ss << TopoFeature::get_obj_f(offset);
  return ss.str();
}


