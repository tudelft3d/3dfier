//
//  Polygon3d.cpp
//  boo
//
//  Created by Hugo Ledoux on 28/08/15.
//  Copyright (c) 2015 Hugo Ledoux. All rights reserved.
//

#include "Polygon3d.h"

Polygon3d::Polygon3d(Polygon2d* p, ExtrusionType type, std::string id) {
  _id = id;
  _p2d = p;
  _type = type;
}

Polygon3d::~Polygon3d() {
  // TODO: clear memory properly
}

Box Polygon3d::get_bbox2d() {
  return bg::return_envelope<Box>(*_p2d);
}


std::string Polygon3d::get_id() {
  return _id;
}

Polygon2d* Polygon3d::get_polygon2d() {
    return _p2d;
}

