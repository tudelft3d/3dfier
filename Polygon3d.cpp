
#include "Polygon3d.h"
#include "input.h"

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
//-------------------------------

Polygon3dBlockSimple::Polygon3dBlockSimple(Polygon2d* p, std::string id, std::string heightref) : Polygon3d(p, id) 
{
  if (_heightref == "MAX")
    _onezvalue = -999999;
  else if (_heightref == "MIN")
    _onezvalue = 999999;
  else
    _onezvalue = 0.0;
}

std::string Polygon3dBlockSimple::get_lift_type() {
  return "BLOCK";
}

std::string Polygon3dBlockSimple::get_3d_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>";
  ss << "<bldg:Building>";
  ss << "<bldg:measuredHeight uom=\"#m\">";
  ss << this->get_height();
  ss << "</bldg:measuredHeight>";
  ss << "<bldg:lod1Solid>";
  ss << "<gml:Solid>";
  ss << "<gml:exterior>";
  ss << "<gml:CompositeSurface>";
  //-- get floor
  ss << get_polygon_lifted_gml(this->_p2d, 0, false);
  //-- get roof
  ss << get_polygon_lifted_gml(this->_p2d, this->get_height(), true);
  //-- get the walls
  auto r = bg::exterior_ring(*(this->_p2d));
  for (int i = 0; i < (r.size() - 1); i++) 
    ss << get_extruded_line_gml(&r[i], &r[i + 1], this->get_height(), 0, false);
  ss << "</gml:CompositeSurface>";
  ss << "</gml:exterior>";
  ss << "</gml:Solid>";
  ss << "</bldg:lod1Solid>";
  ss << "</bldg:Building>";
  ss << "</cityObjectMember>";
  return ss.str(); 
}


double Polygon3dBlockSimple::get_height() {
  // TODO : return an error if 
  if (_no_points == 0)
    return -999;
  if ( (_heightref == "MAX") || (_heightref == "MIN") )
    return _onezvalue;
  else { //-- AVG
    return (_onezvalue / _no_points);
  }
}


bool Polygon3dBlockSimple::add_elevation_point(double x, double y, double z) {
  if (_heightref == "MAX") {
    if (z > _onezvalue)
      _onezvalue = z;
  }
  else if (_heightref == "MIN") {
    if (z < _onezvalue)
      _onezvalue = z;
  }
  else {
    _onezvalue += z;
  }
  return true;
}



//-------------------------------
//-------------------------------

Polygon3dBlockPercentile::Polygon3dBlockPercentile(Polygon2d* p, std::string id, int percentile) : Polygon3d(p, id) 
{
  _percentile = percentile;
}

std::string Polygon3dBlockPercentile::get_lift_type() {
  return "BLOCK_PERCENTILE";
}

std::string Polygon3dBlockPercentile::get_3d_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>";
  ss << "<bldg:Building>";
  ss << "<bldg:measuredHeight uom=\"#m\">";
  ss << this->get_height();
  ss << "</bldg:measuredHeight>";
  ss << "<bldg:lod1Solid>";
  ss << "<gml:Solid>";
  ss << "<gml:exterior>";
  ss << "<gml:CompositeSurface>";
  //-- get floor
  ss << get_polygon_lifted_gml(this->_p2d, 0, false);
  //-- get roof
  ss << get_polygon_lifted_gml(this->_p2d, this->get_height(), true);
  //-- get the walls
  auto r = bg::exterior_ring(*(this->_p2d));
  for (int i = 0; i < (r.size() - 1); i++) 
    ss << get_extruded_line_gml(&r[i], &r[i + 1], this->get_height(), 0, false);
  ss << "</gml:CompositeSurface>";
  ss << "</gml:exterior>";
  ss << "</gml:Solid>";
  ss << "</bldg:lod1Solid>";
  ss << "</bldg:Building>";
  ss << "</cityObjectMember>";
  return ss.str(); 
}


double Polygon3dBlockPercentile::get_height() {
  std::nth_element(_zvalues.begin(), _zvalues.begin() + (_zvalues.size() / 2), _zvalues.end());
  return _zvalues[_zvalues.size() / 2];
}


bool Polygon3dBlockPercentile::add_elevation_point(double x, double y, double z) {
  _zvalues.push_back(z);
  return true;
}
