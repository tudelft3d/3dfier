
#include "Polygon3d.h"
#include "input.h"

Polygon3d::Polygon3d(Polygon2* p, std::string pid) {
  _id = pid;
  _p2 = p;
}

Polygon3d::~Polygon3d() {
  // TODO: clear memory properly
  std::cout << "I am dead" << std::endl;
}

Box Polygon3d::get_bbox2d() {
  return bg::return_envelope<Box>(*_p2);
}

std::string Polygon3d::get_id() {
  return _id;
}

Polygon2* Polygon3d::get_Polygon2() {
    return _p2;
}



//-------------------------------
//-------------------------------

Polygon3dBlock::Polygon3dBlock(Polygon2* p, std::string pid, std::string lifttype) : Polygon3d(p, pid) 
{
  _lifttype = lifttype;
  _vertexelevations.assign(bg::num_points(*(_p2)), 9999);
}

std::string Polygon3dBlock::get_lift_type() {
  return _lifttype;
}

int Polygon3dBlock::get_number_vertices() {
  return int(2* _vertices.size());
}

bool Polygon3dBlock::threeDfy() {
  build_CDT();
  return true;
}

std::string Polygon3dBlock::get_3d_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>";
  ss << "<bldg:Building>";
  ss << "<bldg:measuredHeight uom=\"#m\">";
  ss << this->get_roof_height();
  ss << "</bldg:measuredHeight>";
  ss << "<bldg:lod1Solid>";
  ss << "<gml:Solid>";
  ss << "<gml:exterior>";
  ss << "<gml:CompositeSurface>";
  //-- get floor
  ss << get_polygon_lifted_gml(this->_p2, 0, false);
  //-- get roof
  ss << get_polygon_lifted_gml(this->_p2, this->get_roof_height(), true);
  //-- get the walls
  auto r = bg::exterior_ring(*(this->_p2));
  for (int i = 0; i < (r.size() - 1); i++) 
    ss << get_extruded_line_gml(&r[i], &r[i + 1], this->get_roof_height(), 0, false);
  ss << "</gml:CompositeSurface>";
  ss << "</gml:exterior>";
  ss << "</gml:Solid>";
  ss << "</bldg:lod1Solid>";
  ss << "</bldg:Building>";
  ss << "</cityObjectMember>";
  return ss.str(); 
}

std::string Polygon3dBlock::get_3d_csv() {
  std::stringstream ss;
  ss << this->get_id() << ";" << this->get_roof_height() << ";" << this->get_ground_height() << std::endl;
  return ss.str(); 
}

std::string Polygon3dBlock::get_obj_v() {
  std::stringstream ss;
  for (auto& v : _vertices)
    ss << std::setprecision(2) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << this->get_roof_height() << std::endl;
  for (auto& v : _vertices)
    ss << std::setprecision(2) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << this->get_ground_height() << std::endl;
  return ss.str();
}

std::string Polygon3dBlock::get_obj_f(int offset) {
  std::stringstream ss;
  ss << "usemtl block" << std::endl;
  //-- roof
  for (auto& t : _triangles)
    ss << "f " << (t.v0 + 1 + offset) << " " << (t.v1 + 1 + offset) << " " << (t.v2 + 1 + offset) << std::endl;
  //-- side walls
  for (auto& s : _segments) {
    ss << "f " << (s.v1 + 1 + offset) << " " << (s.v0 + 1 + offset) << " " << (s.v0 + 1 + offset + _vertices.size()) << std::endl;  
    ss << "f " << (s.v0 + 1 + offset + _vertices.size()) << " " << (s.v1 + 1 + offset + _vertices.size()) << " " << (s.v1 + 1 + offset) << std::endl;  
  }
  // //-- ground floor TODO: ground floor of buildings?
  // if (ground == true) {
  //   for (auto& t : _triangles)
  //     ss << "f " << (t.v0 + 1 + offset + _triangles.size() + 2) << " " << (t.v1 + 1 + offset + _triangles.size() + 2) << " " << (t.v2 + 1 + offset + _triangles.size() + 2) << std::endl;  
  // }
  return ss.str();
}

double Polygon3dBlock::get_roof_height() {
  // TODO : return an error if no points
  if (_zvalues.size() == 0)
    return -9999;
  std::string t = _lifttype.substr(_lifttype.find_first_of("-") + 1);
  if (t == "MAX") {
    double v = -9999;
    for (auto z : _zvalues) {
      if (z > v)
        v = z;
    }
    return v;
  }
  else if (t == "MIN") {
    double v = 9999;
    for (auto z : _zvalues) {
      if (z < v)
        v = z;
    }
    return v;
  }
  else if (t == "AVG") {
    double sum = 0.0;
    for (auto z : _zvalues) 
      sum += z;
    return (sum / _zvalues.size());
  }
  else if (t == "MEDIAN") {
    std::nth_element(_zvalues.begin(), _zvalues.begin() + (_zvalues.size() / 2), _zvalues.end());
    return _zvalues[_zvalues.size() / 2];
  }
  else {
    std::cout << "UNKNOWN HEIGHT" << std::endl;
  }
  return -9999;
}


double Polygon3dBlock::get_ground_height() {
  return *(std::min_element(std::begin(_vertexelevations), std::end(_vertexelevations)));
}

bool Polygon3dBlock::add_elevation_point(double x, double y, double z) {
  Point2 p(x, y);
  //-- 1. assign to polygon if inside
  if (bg::within(p, *(_p2)) == true)
    _zvalues.push_back(z);
  //-- 2. add to the vertices of the pgn to find their heights
  assign_elevation_to_vertex(x, y, z);
  return true;
}


//-- TODO: okay here brute-force, use of flann is points>200 (to benchmark perhaps?)  
bool Polygon3dBlock::assign_elevation_to_vertex(double x, double y, double z) {
  //-- assign the MINIMUM to each
  Point2 p(x, y);
  Ring2 oring = bg::exterior_ring(*(_p2));
  for (int i = 0; i < oring.size(); i++) {
    if (bg::distance(p, oring[i]) <= 2.0)
      if (z < _vertexelevations[i])
        _vertexelevations[i] = z;
  }
  auto irings = bg::interior_rings(*(_p2));
  int offset = bg::num_points(*(_p2));
  for (Ring2& iring: irings) {
    for (int i = 0; i < iring.size(); i++) {
      if (bg::distance(p, iring[i]) <= 2.0)
        if (z < _vertexelevations[i + offset])
        _vertexelevations[i + offset] = z;
    }
    offset += bg::num_points(iring);
  }
  return true;
}


bool Polygon3dBlock::build_CDT() {
  getCDT(_p2, _vertices, _triangles, _segments);
  return true;
}


//-------------------------------
//-------------------------------

Polygon3dBoundary::Polygon3dBoundary(Polygon2* p, std::string pid) : Polygon3d(p, pid) 
{
}

std::string Polygon3dBoundary::get_lift_type() {
  return "BOUNDARY3D";
}

bool Polygon3dBoundary::threeDfy() {
  getCDT(_p2, _vertices, _triangles, _segments);
  return true;
}

int Polygon3dBoundary::get_number_vertices() {
  return _vertices.size();
}


std::string Polygon3dBoundary::get_3d_citygml() {
  std::stringstream ss;
  ss << "# vertices: " << _vertices.size() << std::endl;
  ss << "# triangles: " << _triangles.size() << std::endl;
  return ss.str(); 
}

std::string Polygon3dBoundary::get_3d_csv() {
  return "EMPTY"; 
}

std::string Polygon3dBoundary::get_obj_v() {
  std::stringstream ss;
  for (auto& v : _vertices)
    ss << std::setprecision(2) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << bg::get<2>(v) << std::endl;
  return ss.str();
}

std::string Polygon3dBoundary::get_obj_f(int offset) {
  std::stringstream ss;
  ss << "usemtl boundary3d" << std::endl;
  for (auto& t : _triangles)
    ss << "f " << (t.v0 + 1 + offset) << " " << (t.v1 + 1 + offset) << " " << (t.v2 + 1 + offset) << std::endl;
  return ss.str();
}


bool Polygon3dBoundary::add_elevation_point(double x, double y, double z) {
  _lidarpts.push_back(Point3(x, y, z));
  return true;
}


//-------------------------------
//-------------------------------

Polygon3dTin::Polygon3dTin(Polygon2* p, std::string pid, std::string lifttype) : Polygon3d(p, pid) 
{
  _lifttype = lifttype;
  std::string t = lifttype.substr(lifttype.find_first_of("-") + 1);
  if (t == "ALL")
    _thin = 0;
  else 
    _thin = std::stoi(t);
}

std::string Polygon3dTin::get_lift_type() {
  return _lifttype;
}

bool Polygon3dTin::threeDfy() {
  getCDT(_p2, _vertices, _triangles, _segments, _lidarpts);
  return true;
}

int Polygon3dTin::get_number_vertices() {
  return int(_vertices.size());
}


std::string Polygon3dTin::get_3d_citygml() {
  //-- TODO: CityGML implementation for TIN type
  return "EMTPY";
}

std::string Polygon3dTin::get_3d_csv() {
  return "EMPTY"; 
}

std::string Polygon3dTin::get_obj_v() {
  std::stringstream ss;
  for (auto& v : _vertices)
    ss << std::setprecision(2) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << bg::get<2>(v) << std::endl;
  return ss.str();
}

std::string Polygon3dTin::get_obj_f(int offset) {
  std::stringstream ss;
  ss << "usemtl tin" << std::endl;
  for (auto& t : _triangles)
    ss << "f " << (t.v0 + 1 + offset) << " " << (t.v1 + 1 + offset) << " " << (t.v2 + 1 + offset) << std::endl;
  return ss.str();
}


bool Polygon3dTin::add_elevation_point(double x, double y, double z) {
  if (_thin == 0)
    _lidarpts.push_back(Point3(x, y, z));
  else {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, _thin); 
    if (dis(gen) == 1)
      _lidarpts.push_back(Point3(x, y, z));
  }
  return true;
}

