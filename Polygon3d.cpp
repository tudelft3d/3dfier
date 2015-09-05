
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


Polygon3d_block::Polygon3d_block(Polygon2d* p, std::string id, BlockHeightRef heightref) : Polygon3d(p, id) {
  _heightref = heightref;
}

std::string Polygon3d_block::get_3d_citygml() {
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


double Polygon3d_block::get_height() {
  // TODO : return an error if 
  if (_no_points == 0)
    return -999;
  double re;
  switch (_heightref) {
    case AVERAGE: {
      double total = 0.0;
      for (auto z : _zvalues)
        total += z;
      re = total / _no_points;
      break;
    }
    case MEDIAN:
      std::nth_element(_zvalues.begin(), _zvalues.begin() + (_zvalues.size() / 2), _zvalues.end());
      re = _zvalues[_zvalues.size() / 2];
      break;
    case MAXIMUM: {
      double m = -999999;
      for (auto z : _zvalues)
        if (z > m)
          m = z;
      re = m;
      break;
    }
    case MINIMUM: {
      double m = 999999;
      for (auto z : _zvalues)
        if (z < m)
          m = z;
      re = m;
      break;
    }
  }
  return re;
}


bool Polygon3d_block::add_elevation_point(double x, double y, double z) {
  _no_points++;
  _zvalues.push_back(z);
  return true;
}

Lift3DType Polygon3d_block::get_extrusion_type() {
  return BLOCK;
}

BlockHeightRef Polygon3d_block::get_height_reference() {
  return _heightref;
}