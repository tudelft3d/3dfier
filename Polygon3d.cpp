
#include "Polygon3d.h"

Polygon3d::Polygon3d(Polygon2d* p, std::string id) {
  _id = id;
  _p2d = p;
  _no_points = 0;
}

Polygon3d::~Polygon3d() {
  // TODO: clear memory properly
  std::cout << "I am dead" << std::endl;
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


//-------------------------------


Polygon3d_H_AVG::Polygon3d_H_AVG(Polygon2d* p, std::string id) : Polygon3d(p, id) {
  _total_elevation = 0.0;
}

std::string Polygon3d_H_AVG::get_3d_representation() {
  if (this->_no_points > 0)
    return _id;
  else
    return "";
}

bool Polygon3d_H_AVG::add_elevation_point(double x, double y, double z) {
  _no_points++;
  _total_elevation += z;
  return true;
}

ExtrusionType Polygon3d_H_AVG::get_extrusion_type() {
  return HORIZONTAL_AVG;
}

