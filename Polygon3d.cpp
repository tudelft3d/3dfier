
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
    return (std::to_string(_total_elevation / _no_points));
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

std::string Polygon3d_H_AVG::get_citygml() {
  return "avg";
}

//-------------------------------

Polygon3d_H_MEDIAN::Polygon3d_H_MEDIAN(Polygon2d* p, std::string id) : Polygon3d(p, id) {
}

std::string Polygon3d_H_MEDIAN::get_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>";
  ss << "<Building>";
  ss << "<measuredHeight uom=\"#m\">";
  ss << this->get_height();
  ss << "</measuredHeight>";
  ss << "<lod1Solid>";
  ss << "<gml:Solid>";
  ss << "<gml:exterior>";
  ss << "<gml:CompositeSurface>";
  
  ss << "</gml:CompositeSurface>";
  ss << "</gml:exterior>";
  ss << "</gml:Solid>";
  ss << "</lod1Solid>";
  ss << "</Building>";
  ss << "</cityObjectMember>";
  
  return ss.str(); 

  // for (auto)
// <gml:surfaceMember>
// <gml:Polygon>
// <gml:exterior>
// <gml:LinearRing>
// <gml:pos>85105.3311157 446323.558105 0</gml:pos>
}

double Polygon3d_H_MEDIAN::get_height() {
  if (_medianvalues.size() == 0)
    return 0.0;
  else {
    std::nth_element(_medianvalues.begin(), _medianvalues.begin() + (_medianvalues.size() / 2), _medianvalues.end());
    return _medianvalues[_medianvalues.size() / 2];
  }
}

std::string Polygon3d_H_MEDIAN::get_3d_representation() {
  return "median";
}

bool Polygon3d_H_MEDIAN::add_elevation_point(double x, double y, double z) {
  _no_points++;
  _medianvalues.push_back(z);
  return true;
}

ExtrusionType Polygon3d_H_MEDIAN::get_extrusion_type() {
  return HORIZONTAL_MEDIAN;
}
