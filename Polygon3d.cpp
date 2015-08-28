
#include "Polygon3d.h"

Polygon3d::Polygon3d(Polygon2d* p, ExtrusionType type, std::string id) {
  _id = id;
  _p2d = p;
  _extrusiontype = type;
  _total_elevation = 0.0;
  _no_points = 0;
}

Polygon3d::~Polygon3d() {
  // TODO: clear memory properly
  std::cout << "I am dead" << std::endl;
}

Box Polygon3d::get_bbox2d() {
  return bg::return_envelope<Box>(*_p2d);
}

double Polygon3d::get_elevation() {
  if (_no_points == 0)
    return -999;
  else
    return (_total_elevation / _no_points);
}


bool Polygon3d::add_elevation_point(double x, double y, double z) {
  switch(_extrusiontype) {
    case SIMPLE_AVG :
      _no_points++;
      _total_elevation += z;
      break;       
    case SIMPLE_MEDIAN :
      _no_points++;
      _medianvalues.push_back(z);
      break;    
    case TIN_ALL_POINTS :
      _lsPts3d.push_back(new Point3d(x, y, z));
      break;   
    case TIN_SIMPLIFIED :
      // TODO : what are the rules to simplify actually? start with just skipping points I'd say
      break;   
  }
  return true;
}

std::string Polygon3d::get_id() {
  return _id;
}

Polygon2d* Polygon3d::get_polygon2d() {
    return _p2d;
}

