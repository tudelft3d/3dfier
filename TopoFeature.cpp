
#include "TopoFeature.h"
#include "io.h"

//-- initialisation of 
std::string Block::_heightref_top = "median";
std::string Block::_heightref_base = "percentile-10";


//-----------------------------------------------------------------------------

TopoFeature::TopoFeature(Polygon2* p, std::string pid) {
  _id = pid;
  _p2 = p;
  _velevations.resize(bg::num_points(*(_p2)));
}

TopoFeature::~TopoFeature() {
  // TODO: clear memory properly
  std::cout << "I am dead" << std::endl;
}

Box TopoFeature::get_bbox2d() {
  return bg::return_envelope<Box>(*_p2);
}

std::string TopoFeature::get_id() {
  return _id;
}

Polygon2* TopoFeature::get_Polygon2() {
    return _p2;
}

std::string TopoFeature::get_obj_v() {
  std::stringstream ss;
  for (auto& v : _vertices)
    ss << std::setprecision(2) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << bg::get<2>(v) << std::endl;
  return ss.str();
}

std::string TopoFeature::get_obj_f(int offset) {
  std::stringstream ss;
  for (auto& t : _triangles)
    ss << "f " << (t.v0 + 1 + offset) << " " << (t.v1 + 1 + offset) << " " << (t.v2 + 1 + offset) << std::endl;
  return ss.str();
}

bool TopoFeature::assign_elevation_to_vertex(double x, double y, double z, float radius) {
  //-- TODO: okay here brute-force, use of flann is points>200 (to benchmark perhaps?)  
  Point2 p(x, y);
  Ring2 oring = bg::exterior_ring(*(_p2));
  for (int i = 0; i < oring.size(); i++) {
    if (bg::distance(p, oring[i]) <= radius)
      (_velevations[i]).push_back(z);
  }
  int offset = int(bg::num_points(oring));
  auto irings = bg::interior_rings(*(_p2));
  for (Ring2& iring: irings) {
    for (int i = 0; i < iring.size(); i++) {
      if (bg::distance(p, iring) <= radius)
          (_velevations[i + offset]).push_back(z);
    }
    offset += bg::num_points(iring);
  }
  return true;
}

void TopoFeature::lift_vertices_boundary(Polygon3 &p3, float percentile) {
  //-- fetch one value for each vertex
  std::vector<float> zvertices;
  float avg = 0.0;
  for (auto& v : _velevations) {
    if (v.size() > 0) {
      std::nth_element(v.begin(), v.begin() + (v.size() * percentile), v.end());
      zvertices.push_back(v[v.size() * percentile]);
      avg += v[v.size() * percentile];
    }
    else
      zvertices.push_back(-9999);
  }
  avg /= zvertices.size();
  //-- replace missing values by the avg of the polygon TODO : is avg to fix problems the best? I doubt.
  for (auto& v : zvertices) 
    if (v < -9998)
      v = avg;
  //-- assign the value of the Polygon3
  Ring3 oring = bg::exterior_ring(p3);
  for (int i = 0; i < oring.size(); i++) 
    bg::set<2>(bg::exterior_ring(p3)[i], zvertices[i]);
  auto irings = bg::interior_rings(p3);
  std::size_t offset = bg::num_points(oring);
  for (int i = 0; i < irings.size(); i++) {
    for (int j = 0; j < (irings[i]).size(); j++)
      bg::set<2>(bg::interior_rings(p3)[i][j], zvertices[j + offset]);
    offset += bg::num_points(irings[i]);
  }
  _velevations.clear();
  _velevations.shrink_to_fit();
}  



//-------------------------------
//-------------------------------


Block::Block(Polygon2* p, std::string pid, std::string heightref_top, std::string heightref_base) : TopoFeature(p, pid) 
{
  _is3d = false;
  _heightref_top = heightref_top;
  _heightref_base = heightref_base;
}


int Block::get_number_vertices() {
  return int(2 * _vertices.size());
}


std::string Block::get_obj_v() {
  std::stringstream ss;
  for (auto& v : _vertices)
    ss << std::setprecision(2) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << this->get_height_top() << std::endl;
  for (auto& v : _vertices)
    ss << std::setprecision(2) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << this->get_height_base() << std::endl;
  return ss.str();
}

std::string Block::get_obj_f(int offset) {
  std::stringstream ss;
  //-- top surface
  for (auto& t : _triangles)
    ss << "f " << (t.v0 + 1 + offset) << " " << (t.v1 + 1 + offset) << " " << (t.v2 + 1 + offset) << std::endl;
  //-- side surfaces walls
  for (auto& s : _segments) {
    ss << "f " << (s.v1 + 1 + offset) << " " << (s.v0 + 1 + offset) << " " << (s.v0 + 1 + offset + _vertices.size()) << std::endl;  
    ss << "f " << (s.v0 + 1 + offset + _vertices.size()) << " " << (s.v1 + 1 + offset + _vertices.size()) << " " << (s.v1 + 1 + offset) << std::endl;  
  }
  return ss.str();
}


float Block::get_height_top() {
  if (_is3d == true)
    return _height_top;
  if (_zvaluesinside.size() == 0)
    return -9999;
    if (_heightref_top == "max") {
    double v = -9999;
    for (auto z : _zvaluesinside) {
      if (z > v)
        v = z;
    }
    return v;
  }
  else if (_heightref_top == "min") {
    double v = 9999;
    for (auto z : _zvaluesinside) {
      if (z < v)
        v = z;
    }
    return v;
  }
  else if (_heightref_top == "avg") {
    double sum = 0.0;
    for (auto z : _zvaluesinside) 
      sum += z;
    return (sum / _zvaluesinside.size());
  }
  else if (_heightref_top == "median") {
    std::nth_element(_zvaluesinside.begin(), _zvaluesinside.begin() + (_zvaluesinside.size() / 2), _zvaluesinside.end());
    return _zvaluesinside[_zvaluesinside.size() / 2];
  }
  else {
    if (_heightref_top.substr(0, _heightref_top.find_first_of("-")) == "percentile") {
      double p = (std::stod(_heightref_top.substr(_heightref_top.find_first_of("-") + 1)) / 100);
      std::nth_element(_zvaluesinside.begin(), _zvaluesinside.begin() + (_zvaluesinside.size() * p), _zvaluesinside.end());
      return _zvaluesinside[_zvaluesinside.size() * p];
    }
    else
      std::cerr << "ERROR: height reference '" << _heightref_top << "' unknown." << std::endl;
  }
  return -9999;
}


float Block::get_height_base() {
  if (_is3d == true)
    return _height_base;
  double p = (std::stod(_heightref_base.substr(_heightref_base.find_first_of("-") + 1)) / 100);
  std::vector<float> tmp;
  for (auto& each : _velevations) {
    if (each.size() > 0) {
      std::nth_element(each.begin(), each.begin() + (each.size() * p), each.end());
      tmp.push_back(each[each.size() * p]);
    }
  }
  return *(std::min_element(std::begin(tmp), std::end(tmp)));
}


bool Block::add_elevation_point(double x, double y, double z, float radius) {
  Point2 p(x, y);
  //-- 1. assign to polygon if inside
  if (bg::within(p, *(_p2)) == true)
    _zvaluesinside.push_back(z);
  //-- 2. add to the vertices of the pgn to find their heights
  assign_elevation_to_vertex(x, y, z, radius);
  return true;
}


bool Block::build_CDT() {
  std::stringstream ss;
  ss << bg::wkt(*(_p2));
  Polygon3 p3;
  bg::read_wkt(ss.str(), p3);
  getCDT(&p3, _vertices, _triangles, _segments);
  return true;
}


//-------------------------------
//-------------------------------

Boundary3D::Boundary3D(Polygon2* p, std::string pid) : TopoFeature(p, pid) 
{}


int Boundary3D::get_number_vertices() {
  return int(_vertices.size());
}


bool Boundary3D::add_elevation_point(double x, double y, double z, float radius) {
  assign_elevation_to_vertex(x, y, z, radius);
  return true;
}

void Boundary3D::smooth_boundary(Polygon3 &p3, int passes) {
  for (int p = 0; p < passes; p++) {
    Ring3 oring = bg::exterior_ring(p3);
    std::vector<float> elevs(bg::num_points(oring));
    smooth_ring(oring, elevs);
    for (int i = 0; i < oring.size(); i++) 
      bg::set<2>(bg::exterior_ring(p3)[i], elevs[i]);
    auto irings = bg::interior_rings(p3);
    for (int i = 0; i < irings.size(); i++) {
      elevs.resize(bg::num_points(irings[i]));
      smooth_ring(irings[i], elevs);
      for (int j = 0; j < (irings[i]).size(); j++)
        bg::set<2>(bg::interior_rings(p3)[i][j], elevs[j]);
    }
  }
}

void Boundary3D::smooth_ring(Ring3 &r, std::vector<float> &elevs) {
  elevs.front() = (bg::get<2>(r[1]) + bg::get<2>(r.back())) / 2;
  auto it = r.end();
  it -= 2;
  elevs.back() = (bg::get<2>(r.front()) + bg::get<2>(*it)) / 2;
  for (int i = 1; i < (r.size() - 1); i++) 
    elevs[i] = (bg::get<2>(r[i - 1]) + bg::get<2>(r[i + 1])) / 2;
}



//-------------------------------
//-------------------------------

TIN::TIN(Polygon2* p, std::string pid, int simplification) : TopoFeature(p, pid) 
{
  _simplification = simplification;
}


int TIN::get_number_vertices() {
  return int(_vertices.size());
}


bool TIN::add_elevation_point(double x, double y, double z, float radius) {
  Point2 p(x, y);
  //-- 1. add points to surface if inside
  if (bg::within(p, *(_p2)) == true) {
    if (_simplification == 0)
      _lidarpts.push_back(Point3(x, y, z));
    else {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<int> dis(1, _simplification);
      if (dis(gen) == 1)
        _lidarpts.push_back(Point3(x, y, z));
    }
  }
  //-- 2. add to the vertices of the pgn to find their heights
  assign_elevation_to_vertex(x, y, z, radius);
  return true;
}



