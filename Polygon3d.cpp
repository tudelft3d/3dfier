
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


Polygon3d_SA::Polygon3d_SA(Polygon2d* p, std::string id) {
  _id = id;
  _p2d = p;
  _no_points = 0;
  _total_elevation = 0.0;
}


double Polygon3d_SA::get_elevation() {
  if (_no_points == 0)
    return -999;
  else
    return (_total_elevation / _no_points);
}


bool Polygon3d_SA::add_elevation_point(double x, double y, double z) {
  _no_points++;
  _total_elevation += z;
  return true;
}


