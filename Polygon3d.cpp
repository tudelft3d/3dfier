
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

std::string Polygon3d::get_polygon_lifted_gml(Polygon2d* p2, double height, bool reverse) {
  std::stringstream ss;
  ss << "<gml:surfaceMember>";
  ss << "<gml:Polygon>";
  ss << "<gml:exterior>";
  ss << "<gml:LinearRing>";
  if (reverse)
    bg::reverse(*p2);
  // TODO : also do the interior rings for extrusion
  auto r = bg::exterior_ring(*p2);
  for (int i = 0; i < r.size(); i++)
    ss << "<gml:pos>" << bg::get<0>(r[i]) << " " << bg::get<1>(r[i]) << " " << height << "</gml:pos>";
  ss << "</gml:LinearRing>";
  ss << "</gml:exterior>";
  ss << "</gml:Polygon>";
  ss << "</gml:surfaceMember>";
  if (reverse)
    bg::reverse(*p2);
  return ss.str();
}

std::string Polygon3d::get_extruded_line_gml(Point2d* a, Point2d* b, double high, double low, bool reverse) {
  std::stringstream ss;
  ss << "<gml:surfaceMember>";
  ss << "<gml:Polygon>";
  ss << "<gml:exterior>";
  ss << "<gml:LinearRing>";
  ss << "<gml:pos>" << bg::get<0>(b) << " " << bg::get<1>(b) << " " << low << "</gml:pos>";
  ss << "<gml:pos>" << bg::get<0>(a) << " " << bg::get<1>(a) << " " << low << "</gml:pos>";
  ss << "<gml:pos>" << bg::get<0>(a) << " " << bg::get<1>(a) << " " << high << "</gml:pos>";
  ss << "<gml:pos>" << bg::get<0>(b) << " " << bg::get<1>(b) << " " << high << "</gml:pos>";
  ss << "<gml:pos>" << bg::get<0>(b) << " " << bg::get<1>(b) << " " << low << "</gml:pos>";
  ss << "</gml:LinearRing>";
  ss << "</gml:exterior>";
  ss << "</gml:Polygon>";
  ss << "</gml:surfaceMember>";
  return ss.str();
}

std::string Polygon3d::get_extruded_lod1_block_gml(Polygon2d* p2, double high, double low) {
  std::stringstream ss;
  //-- get floor
  ss << this->get_polygon_lifted_gml(p2, low, false);
  //-- get roof
  ss << this->get_polygon_lifted_gml(p2, high, true);
  //-- get the walls
  auto r = bg::exterior_ring(*p2);
  for (int i = 0; i < (r.size() - 1); i++) 
    ss << get_extruded_line_gml(&r[i], &r[i + 1], high, low, false);
  return ss.str();
}

//-------------------------------
//-------------------------------


Polygon3d_H_AVG::Polygon3d_H_AVG(Polygon2d* p, std::string id) : Polygon3d(p, id) {
  _total_elevation = 0.0;
}

double Polygon3d_H_AVG::get_height() {
 if (this->_no_points > 0)
   return double(_total_elevation / _no_points);
 else
   return -999.0;
}

bool Polygon3d_H_AVG::add_elevation_point(double x, double y, double z) {
  _no_points++;
  _total_elevation += z;
  return true;
}

ExtrusionType Polygon3d_H_AVG::get_extrusion_type() {
  return HORIZONTAL_AVG;
}

std::string Polygon3d_H_AVG::get_3d_citygml() {
  std::stringstream ss;
  //-- get floor
  ss << this->get_polygon_lifted_gml(this->_p2d, 0, false);
  //-- get roof
//  ss << this->get_polygon_lifted_gml(this->_p2d, , true);
  //-- get the walls
//  auto r = bg::exterior_ring(*p2);
//  for (int i = 0; i < (r.size() - 1); i++)
//    ss << get_extruded_line_gml(&r[i], &r[i + 1], high, low, false);
  return ss.str();
}

//-------------------------------

Polygon3d_H_MEDIAN::Polygon3d_H_MEDIAN(Polygon2d* p, std::string id) : Polygon3d(p, id) {
}

std::string Polygon3d_H_MEDIAN::get_3d_citygml() {
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
  //-- get floor
  ss << this->get_polygon_lifted_gml(this->_p2d, 0, false);
  //-- get roof
  ss << this->get_polygon_lifted_gml(this->_p2d, this->get_height(), true);
  //-- get the walls
  auto r = bg::exterior_ring(*(this->_p2d));
  for (int i = 0; i < (r.size() - 1); i++) 
    ss << get_extruded_line_gml(&r[i], &r[i + 1], this->get_height(), 0, false);
  ss << "</gml:CompositeSurface>";
  ss << "</gml:exterior>";
  ss << "</gml:Solid>";
  ss << "</lod1Solid>";
  ss << "</Building>";
  ss << "</cityObjectMember>";
  return ss.str(); 
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
