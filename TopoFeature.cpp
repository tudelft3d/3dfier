/*
 Copyright (c) 2015 Hugo Ledoux
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/
 
#include "TopoFeature.h"
#include "io.h"

//-- initialisation of 
int TopoFeature::_count = 0;
std::string Block::_heightref_top = "percentile-50";
std::string Block::_heightref_base = "percentile-10";


//-----------------------------------------------------------------------------

TopoFeature::TopoFeature(char *wkt, std::string pid) {
  _id = pid;
  _counter = _count++;
  _toplevel = true;
  _p2 = new Polygon2();
  bg::read_wkt(wkt, *(_p2));
  bg::read_wkt(wkt, _p3);
  _velevations.resize(bg::num_points(*(_p2)));
  _nc.resize(bg::num_points(*(_p2)));
}

TopoFeature::~TopoFeature() {
  // TODO: clear memory properly
  std::cout << "I am dead" << std::endl;
}

Box2 TopoFeature::get_bbox2d() {
  return bg::return_envelope<Box2>(*_p2);
}

std::string TopoFeature::get_id() {
  return _id;
}

int TopoFeature::get_counter() {
  return _counter;
}

bool TopoFeature::get_top_level() {
  return _toplevel;
}

void TopoFeature::set_top_level(bool toplevel) {
  _toplevel = toplevel;
}

Polygon2* TopoFeature::get_Polygon2() {
    return _p2;
}

std::string TopoFeature::get_obj_v(int z_exaggeration) {
  std::stringstream ss;
  for (auto& v : _vertices)
    ss << std::setprecision(3) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << (z_exaggeration > 0? (z_exaggeration * bg::get<2>(v)) : bg::get<2>(v)) << std::endl;
  for (auto& v : _vertices_vw)
    ss << std::setprecision(3) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << (z_exaggeration > 0? (z_exaggeration * bg::get<2>(v)) : bg::get<2>(v)) << std::endl;
  return ss.str();
}

std::string TopoFeature::get_obj_f(int offset, bool usemtl) {
  if (this->get_id() == "116727828") {
    std::clog << "116727828" << std::endl;
    std::clog << "nc size: " << _nc.size() << std::endl;
    std::clog << "_vertices_vw: " << _vertices_vw.size() << std::endl;
    std::clog << "_triangles_vw: " <<_triangles_vw.size() << std::endl;
  }
  std::stringstream ss;
  for (auto& t : _triangles)
    ss << "f " << (t.v0 + 1 + offset) << " " << (t.v1 + 1 + offset) << " " << (t.v2 + 1 + offset) << std::endl;
  unsigned long k = _vertices.size();
  if (usemtl == true)
    ss << "usemtl VerticalWalls" << std::endl;
  for (auto& t : _triangles_vw)
    ss << "f " << (t.v0 + 1 + offset + k) << " " << (t.v1 + 1 + offset + k) << " " << (t.v2 + 1 + offset + k) << std::endl;
  return ss.str();
}


void TopoFeature::construct_vertical_walls(std::vector<TopoFeature*> lsAdj) {
  if (this->get_id() == "111114973") {
    std::clog << "-- 111114973 --" << std::endl;
    std::clog << "lsAdj: " << lsAdj.size() << std::endl;
    for (auto& f : lsAdj) 
      std::clog << f->get_id() << " " << std::endl;
    std::clog << std::endl;
  }
  int i = 0;
  for (auto& curnc : _nc) {
    if (curnc.empty() == false) {
      curnc.push_back(this->get_point_elevation(i));
      std::sort(curnc.begin(), curnc.end());
    }
    i++;
  }

  int ai = 0;
  int bi;
  Point2 a;
  Point2 b;
  TopoFeature* fadj;
  for (auto& curnc : _nc) {
    fadj = nullptr;
    b = this->get_next_point2_in_ring(ai, bi);
    std::vector<float> nnc = _nc[bi];
    if ( (curnc.size() > 0) || (nnc.size() > 0) ) {
      get_point2(ai, a);
      // std::clog << "a: " << bg::get<0>(a) << " " << bg::get<1>(a) << std::endl;
      // std::clog << "b: " << bg::get<0>(b) << " " << bg::get<1>(b) << std::endl;
      for (auto& f : lsAdj) {
        // std::clog << f->get_id() << std::endl;
        if (f->has_segment(b, a) == true) {
          fadj = f;
          break;
        }
      }
      if (fadj != nullptr) {
        // if ( (std::abs(this->get_point_elevation(ai) - fadj->get_point_elevation(fadj->has_point2(a))) > 0.01) ||
        //      (std::abs(this->get_point_elevation(bi) - fadj->get_point_elevation(fadj->has_point2(b))) > 0.01) )
        double fadj_az = fadj->get_point_elevation(fadj->has_point2(a));
        double fadj_bz = fadj->get_point_elevation(fadj->has_point2(b));
        double adiff = std::abs(this->get_point_elevation(ai) - fadj_az);
        double bdiff = std::abs(this->get_point_elevation(bi) - fadj_bz);
        if (this->get_id() == "111114973") {
          std::clog << "fadj: " << fadj->get_id() << std::endl;
          std::clog << "size: " << curnc.size() << " : " << nnc.size() << std::endl;
          std::clog << adiff << std::endl; 
          std::clog << adiff << std::endl; 
        }
        if ( (adiff > 0.01) || (bdiff > 0.01) ) 
        {
          if (nnc.size() > 0) {
            for (int j = 0; j < (nnc.size() - 1); j++) {
              if (nnc[j] >= fadj_bz) {
                _vertices_vw.push_back(Point3(this->get_point_x(ai), this->get_point_y(ai), this->get_point_elevation(ai)));
                _vertices_vw.push_back(Point3(this->get_point_x(bi), this->get_point_y(bi), nnc[j]));
                _vertices_vw.push_back(Point3(this->get_point_x(bi), this->get_point_y(bi), nnc[j + 1]));
                Triangle t;
                t.v0 = int(_vertices_vw.size()) - 3;
                t.v1 = int(_vertices_vw.size()) - 1;
                t.v2 = int(_vertices_vw.size()) - 2;
                _triangles_vw.push_back(t);
                if (this->get_id() == "111114973")
                  std::clog << "vw added" << std::endl;
              }
            }
          }
          if (curnc.size() > 0) {
            for (int j = 0; j < (curnc.size() - 1); j++) {
              if (curnc[j] >= fadj_az) {
                if (nnc.size() == 0) {
                  _vertices_vw.push_back(Point3(this->get_point_x(bi), this->get_point_y(bi), this->get_point_elevation(bi)));
                }
                else {
                  _vertices_vw.push_back(Point3(this->get_point_x(bi), this->get_point_y(bi), nnc.front()));
                }
                _vertices_vw.push_back(Point3(this->get_point_x(ai), this->get_point_y(ai), curnc[j]));
                _vertices_vw.push_back(Point3(this->get_point_x(ai), this->get_point_y(ai), curnc[j + 1]));
                Triangle t;
                t.v0 = int(_vertices_vw.size()) - 3;
                t.v1 = int(_vertices_vw.size()) - 2;
                t.v2 = int(_vertices_vw.size()) - 1;
                _triangles_vw.push_back(t);
                if (this->get_id() == "111114973")
                  std::clog << "vw added" << std::endl;
              }
            }
          }
        }
      }
    }
    ai++;
  }
  if (this->get_id() == "111114973")
    std::clog << "-- 111114973 --" << std::endl;

}
    

bool TopoFeature::has_segment(Point2& a, Point2& b) {
  double threshold = 0.001;
  int posa = this->has_point2(a);
  if (posa != -1) {
    Point2 tmp;
    int itmp;
    tmp = this->get_next_point2_in_ring(posa, itmp);
    // if (bg::equals(b, tmp) == true) {
    std::clog << bg::distance(b, tmp) << std::endl;
    if (bg::distance(b, tmp) <= threshold)
      return true;
  }
  return false;
}


int TopoFeature::has_point2(Point2& p) {
  double threshold = 0.001;
  Ring2 oring = bg::exterior_ring(*_p2);
  int re = -1;
  for (int i = 0; i < oring.size(); i++) {
    if (bg::distance(p, oring[i]) <= threshold)
      return i;
  }
  int offset = int(bg::num_points(oring));
  auto irings = bg::interior_rings(*_p2);
  for (Ring2& iring: irings) {
    for (int i = 0; i < iring.size(); i++) {
      if (bg::distance(p, iring[i]) <= threshold)
        return (i + offset);
    }
    offset += bg::num_points(iring);
  }
  return re;
}

Point2 TopoFeature::get_previous_point2_in_ring(int i, int& index) {
  Ring2 oring = bg::exterior_ring(*_p2);
  int offset = int(bg::num_points(oring));
  if (i < offset) {
    if (i == 0) {
      index = (offset - 1);
      return oring.back();
    }
    else {
      index = (i - 1);
      return oring[i - 1];
    }
  }
  for (Ring2& iring: bg::interior_rings(*_p2)) {
    if (i < (offset + iring.size())) {
      if (i == offset) {
        index = i;
        return iring.back();
      }
      else {
        index = (offset + i - 1);
        return (iring[i - 1 - offset]);
      }
    }
    offset += iring.size();
  }
  Point2 tmp;
  return tmp;
}


Point2 TopoFeature::get_next_point2_in_ring(int i, int& index) {
  Ring2 oring = bg::exterior_ring(*_p2);
  int offset = int(bg::num_points(oring));
  //-- on the oring
  if (i < offset) { 
    if (i == (offset - 1)) {
      index = 0;
      return oring.front();
    }
    else {
      index = (i + 1);
      return oring[i + 1];
    }
  }
  //-- not on the oring: search irings
  for (Ring2& iring: bg::interior_rings(*_p2)) {
    if (i < (offset + iring.size())) {
      if (i == (offset - 1 + iring.size())) {
        index = offset;
        return iring.front();
      }
      else {
        index = (i + 1);
        return (iring[i + 1 - offset]);
      }
    }
    offset += iring.size();
  }
  Point2 tmp;
  return tmp;
}


void TopoFeature::add_nc(int i, float z) {

  _nc[i].push_back(z);
}


bool TopoFeature::has_vertical_walls() {
  for (auto& curnc : _nc) 
    if (curnc.size() > 0) 
      return true;
  return false;
}

std::vector<float>& TopoFeature::get_nc(int i) {
  return _nc[i];
}


void TopoFeature::get_point2(int i, Point2& p) {
  Ring2 oring = bg::exterior_ring(*(_p2));
  int offset = int(bg::num_points(oring));
  if (i < offset) {
    p = oring[i];
    return;
  }
  for (Ring2& iring: bg::interior_rings(*(_p2))) {
    if (i < (offset + iring.size())) {
      p = iring[i - offset];
      return;
    }
    offset += iring.size();
  }
  return;
}


double TopoFeature::get_point_x(int i) {
  Ring2 oring = bg::exterior_ring(*(_p2));
  int offset = int(bg::num_points(oring));
  if (i < offset) 
    return bg::get<0>(oring[i]);
  for (Ring2& iring: bg::interior_rings(*(_p2))) {
    if (i < (offset + iring.size()))
      return bg::get<0>(iring[i - offset]);
    offset += iring.size();
  }
  return 0.0;
}


double TopoFeature::get_point_y(int i) {
  Ring2 oring = bg::exterior_ring(*(_p2));
  int offset = int(bg::num_points(oring));
  if (i < offset) 
    return bg::get<1>(oring[i]);
  for (Ring2& iring: bg::interior_rings(*(_p2))) {
    if (i < (offset + iring.size()))
      return bg::get<1>(iring[i - offset]);
    offset += iring.size();
  }
  return 0.0;
}


float TopoFeature::get_point_elevation(int i) {
  Ring3 oring = bg::exterior_ring(_p3);
  int offset = int(bg::num_points(oring));
  if (i < offset) 
    return bg::get<2>(oring[i]);
  for (Ring3& iring: bg::interior_rings(_p3)) {
    if (i < (offset + iring.size()))
      return bg::get<2>(iring[i - offset]);
    offset += iring.size();
  }
  return 0.0;
}

void TopoFeature::set_point_elevation(int i, float z) {
  Ring3 oring = bg::exterior_ring(_p3);
  int offset = int(bg::num_points(oring));
  if (i < offset) {
    bg::set<2>(bg::exterior_ring(_p3)[i], z);
    return;
  }
  auto irings = bg::interior_rings(_p3);
  for (int j = 0; j < irings.size(); j++) {
    if (i < (offset + irings[j].size()))
      bg::set<2>(bg::interior_rings(_p3)[j][i - offset], z);
    offset += irings[j].size();
  }    
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
      if (bg::distance(p, iring) <= radius) {
        (_velevations[i + offset]).push_back(z);
      }
    }
    offset += bg::num_points(iring);
  }
  return true;
}

void TopoFeature::lift_vertices_boundary(float percentile) {
  //-- fetch one average value to assign to missing ones
  // TODO : is avg to fix problems the best? I doubt.
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
  for (auto& v : zvertices) 
    if (v < -9998) {
      v = avg;
      // std::clog << "** interpolation vertex ** " << this->get_class() << std::endl;
    }
  //-- assign the value of the Polygon3
  Ring3 oring = bg::exterior_ring(_p3);
  for (int i = 0; i < oring.size(); i++) 
    bg::set<2>(bg::exterior_ring(_p3)[i], zvertices[i]);
  auto irings = bg::interior_rings(_p3);
  std::size_t offset = bg::num_points(oring);
  for (int i = 0; i < irings.size(); i++) {
    for (int j = 0; j < (irings[i]).size(); j++)
      bg::set<2>(bg::interior_rings(_p3)[i][j], zvertices[j + offset]);
    offset += bg::num_points(irings[i]);
  }
  _velevations.clear();
  _velevations.shrink_to_fit();
}  



//-------------------------------
//-------------------------------


Block::Block(char *wkt, std::string pid, std::string heightref_top, std::string heightref_base) 
: TopoFeature(wkt, pid) 
{
  _is3d = false;
  _heightref_top = heightref_top;
  _heightref_base = heightref_base;
}


int Block::get_number_vertices() {
  return int(2 * _vertices.size());
}


std::string Block::get_obj_v(int z_exaggeration) {
  std::stringstream ss;
  for (auto& v : _vertices)
    ss << std::setprecision(3) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << (z_exaggeration > 0? (z_exaggeration * this->get_height_top()) : this->get_height_top()) << std::endl;
  for (auto& v : _vertices)
    ss << std::setprecision(3) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << (z_exaggeration > 0? (z_exaggeration * this->get_height_base()) : this->get_height_base()) << std::endl;
  return ss.str();
}

std::string Block::get_obj_f(int offset, bool usemtl) {
  std::stringstream ss;
  //-- top surface
  for (auto& t : _triangles)
    ss << "f " << (t.v0 + 1 + offset) << " " << (t.v1 + 1 + offset) << " " << (t.v2 + 1 + offset) << std::endl;
  
//-- extract segments
  std::vector<Segment> allsegments;
  for (auto& curt : _triangles) {
    bool issegment = false;
    for (auto& t : _triangles) {
      if (triangle_contains_segment(t, curt.v1, curt.v0) == true) {
        issegment = true;
        break;
      }
    }
    if (issegment == false) {
      Segment s;
      s.v0 = curt.v0;
      s.v1 = curt.v1;
      allsegments.push_back(s);
    }
    issegment = false;
    for (auto& t : _triangles) {
      if (triangle_contains_segment(t, curt.v2, curt.v1) == true) {
        issegment = true;
        break;
      }
    }
    if (issegment == false) {
      Segment s;
      s.v0 = curt.v1;
      s.v1 = curt.v2;
      allsegments.push_back(s);
    }
    issegment = false;
    for (auto& t : _triangles) {
      if (triangle_contains_segment(t, curt.v0, curt.v2) == true) {
        issegment = true;
        break;
      }
    }
    if (issegment == false) {
      Segment s;
      s.v0 = curt.v2;
      s.v1 = curt.v0;
      allsegments.push_back(s);
    }
  }
  //-- side surfaces walls
  for (auto& s : allsegments) {
    ss << "f " << (s.v1 + 1 + offset) << " " << (s.v0 + 1 + offset) << " " << (s.v0 + 1 + offset + _vertices.size()) << std::endl;  
    ss << "f " << (s.v0 + 1 + offset + _vertices.size()) << " " << (s.v1 + 1 + offset + _vertices.size()) << " " << (s.v1 + 1 + offset) << std::endl;  
  }
  return ss.str();
}


float Block::get_height_top() {
  if (_is3d == true)
    return _height_top;
  if (_zvaluesinside.size() == 0)
    return 0;
  if (_heightref_top == "max") {
    double v = 0;
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
  return 0;
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
  if (tmp.empty())
    return 0.0;
  else
  return *(std::min_element(std::begin(tmp), std::end(tmp)));
}


bool Block::add_elevation_point(double x, double y, double z, float radius, bool lastreturn) {
  Point2 p(x, y);
  //-- 1. assign to polygon if inside
  if (bg::within(p, *(_p2)) == true)
    _zvaluesinside.push_back(z);
  //-- 2. add to the vertices of the pgn to find their heights
  assign_elevation_to_vertex(x, y, z, radius);
  return true;
}





//-------------------------------
//-------------------------------

Boundary3D::Boundary3D(char *wkt, std::string pid) 
: TopoFeature(wkt, pid) 
{}


int Boundary3D::get_number_vertices() {
  return ( int(_vertices.size()) + int(_vertices_vw.size()) );
}


bool Boundary3D::add_elevation_point(double x, double y, double z, float radius, bool lastreturn) {
  assign_elevation_to_vertex(x, y, z, radius);
  return true;
}

void Boundary3D::smooth_boundary(int passes) {
  for (int p = 0; p < passes; p++) {
    Ring3 oring = bg::exterior_ring(_p3);
    std::vector<float> elevs(bg::num_points(oring));
    smooth_ring(oring, elevs);
    for (int i = 0; i < oring.size(); i++) 
      bg::set<2>(bg::exterior_ring(_p3)[i], elevs[i]);
    auto irings = bg::interior_rings(_p3);
    for (int i = 0; i < irings.size(); i++) {
      elevs.resize(bg::num_points(irings[i]));
      smooth_ring(irings[i], elevs);
      for (int j = 0; j < (irings[i]).size(); j++)
        bg::set<2>(bg::interior_rings(_p3)[i][j], elevs[j]);
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

TIN::TIN(char *wkt, std::string pid, int simplification) 
: TopoFeature(wkt, pid) 
{
  _simplification = simplification;
}


int TIN::get_number_vertices() {
  return ( int(_vertices.size()) + int(_vertices_vw.size()) );
}


bool TIN::add_elevation_point(double x, double y, double z, float radius, bool lastreturn) {
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
  if (lastreturn == true)
    assign_elevation_to_vertex(x, y, z, radius);
  return true;
}



