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
//std::string Flat::_heightref = "percentile-50";


//-----------------------------------------------------------------------------

TopoFeature::TopoFeature(char *wkt, std::string pid) {
  _id = pid;
  _counter = _count++;
  _toplevel = true;
  _p2 = new Polygon2();
  bg::read_wkt(wkt, *(_p2));
  
  //-- TO DELETE
  bg::read_wkt(wkt, _p3);
  _nc.resize(bg::num_points(*(_p2)));
  
  _p2z.resize(bg::num_interior_rings(*_p2) + 1);
  _p2z[0].resize(bg::num_points(_p2->outer()));
  _lidarelevs.resize(bg::num_interior_rings(*_p2) + 1);
  _lidarelevs[0].resize(bg::num_points(_p2->outer()));
  for (int i = 0; i < bg::num_interior_rings(*_p2); i++) {
    _p2z[i+1].resize(bg::num_points(_p2->inners()[i]));
    _lidarelevs[i+1].resize(bg::num_points(_p2->inners()[i]));
  }
}

TopoFeature::~TopoFeature() {
  // TODO: clear memory properly
  std::cout << "I am dead now." << std::endl;
}

Box2 TopoFeature::get_bbox2d() {
  return bg::return_envelope<Box2>(*_p2);
}

std::string TopoFeature::get_id() {
  return _id;
}

bool TopoFeature::buildCDT() {
  getCDT(_p2, _p2z, _vertices, _triangles);
  return true;
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


void TopoFeature::fix_bowtie(std::vector<TopoFeature*> lsAdj) {
  // if (this->get_id() == "116724964") 
  //   std::clog << "--116724964" << std::endl;
  // int ai = 0;
  // int bi;
  // Point2 a;
  // Point2 b;
  // TopoFeature* fadj;
  // for (auto& curnc : _nc) {
  //   fadj = nullptr;
  //   b = this->get_next_point2_in_ring(ai, bi);
  //   std::vector<float> nnc = _nc[bi];
  //   if ( (curnc.size() > 0) || (nnc.size() > 0) ) {
  //     get_point2(ai, a);
  //     for (auto& f : lsAdj) {
  //       if (f->has_segment(b, a) == true) {
  //         fadj = f;
  //         break;
  //       }
  //     }
  //     if ( (fadj != nullptr) && (this->get_counter() < fadj->get_counter()) ) {
  //       double f_az = this->get_point_elevation(ai);
  //       double f_bz = this->get_point_elevation(bi);
  //       double fadj_az = fadj->get_point_elevation(fadj->has_point2(a));
  //       double fadj_bz = fadj->get_point_elevation(fadj->has_point2(b));
  //       if ( ((f_az > fadj_az) && (f_bz < fadj_bz)) || ((f_az < fadj_az) && (f_bz > fadj_bz)) ) {
  //         std::clog << "BOWTIE:" << this->get_id() << " " << fadj->get_id() << std::endl;
  //         std::clog << this->get_class() << " " << fadj->get_class() << std::endl;
  //         // TODO : remove the nc because vertices are lowered
  //         if (this->get_class() > fadj->get_class()) {
  //           this->set_point_elevation(ai, fadj_az);
  //           this->set_point_elevation(bi, fadj_bz);
  //         }
  //         else {
  //           fadj->set_point_elevation(fadj->has_point2(a), f_az);
  //           fadj->set_point_elevation(fadj->has_point2(b), f_bz);
  //         }

  //       }
  //     }
  //   }
  //   ai++;
  // }
}


void TopoFeature::construct_vertical_walls(std::vector<TopoFeature*> lsAdj) {
//   if (this->get_id() == "107729942") {
//     std::clog << "-- 107729942 --" << std::endl;
//     // std::clog << "lsAdj: " << lsAdj.size() << std::endl;
//     // for (auto& f : lsAdj) 
//       // std::clog << f->get_id() << " " << std::endl;
//     // std::clog << std::endl;
//   }
//   float TOL = 0.01;
//   int i = 0;
//   for (auto& curnc : _nc) {
//     if (curnc.empty() == false) {
//       curnc.push_back(this->get_point_elevation(i));
//       std::sort(curnc.begin(), curnc.end());
//       std::reverse(curnc.begin(), curnc.end()); //-- heights from top to bottom 
//     }
//     i++;
//   }

//   int ai = 0;
//   int bi;
//   Point2 a;
//   Point2 b;
//   TopoFeature* fadj;
//   for (auto& curnc : _nc) {
//     fadj = nullptr;
//     b = this->get_next_point2_in_ring(ai, bi);
//     std::vector<float> nnc = _nc[bi];
//     if ( (curnc.size() > 0) || (nnc.size() > 0) ) {
//       get_point2(ai, a);
//       // std::clog << "a: " << bg::get<0>(a) << " " << bg::get<1>(a) << std::endl;
//       // std::clog << "b: " << bg::get<0>(b) << " " << bg::get<1>(b) << std::endl;
//       for (auto& f : lsAdj) {
// //         std::clog << f->get_id() << std::endl;
//         if (f->has_segment(b, a) == true) {
//           fadj = f;
//           break;
//         }
//       }
//       if (fadj != nullptr) {
//         double fadj_az = fadj->get_point_elevation(fadj->has_point2(a));
//         double fadj_bz = fadj->get_point_elevation(fadj->has_point2(b));
//         double adiff = std::abs(this->get_point_elevation(ai) - fadj_az);
//         double bdiff = std::abs(this->get_point_elevation(bi) - fadj_bz);
//         // if (this->get_id() == "111114973") {
//         //   std::clog << "fadj: " << fadj->get_id() << std::endl;
//         //   std::clog << "size: " << curnc.size() << " : " << nnc.size() << std::endl;
//         //   std::clog << adiff << std::endl; 
//         //   std::clog << adiff << std::endl; 
//         // }
//         if ( (adiff > 0.01) || (bdiff > 0.01) ) 
//         {
//           int sa = 0;
//           int ea = 0;
//           for (int j = 0; j < curnc.size(); j++) {
//             if (std::abs(this->get_point_elevation(ai) - curnc[j]) < TOL)
//               sa = j;
//             if (std::abs(fadj_az - curnc[j]) < TOL) 
//               ea = j;
//           }
//           int sb = 0;
//           int eb = 0;
//           for (int j = 0; j < nnc.size(); j++) {
//             if (std::abs(this->get_point_elevation(bi) - nnc[j]) < TOL)
//               sb = j;
//             if (std::abs(fadj_bz - nnc[j]) < TOL) 
//               eb = j;
//           }

//           if (nnc.size() > 0) {
//             for (int j = sb; j < eb; j++) {
//               if (curnc.size() == 0)
//                 _vertices_vw.push_back(Point3(this->get_point_x(ai), this->get_point_y(ai), this->get_point_elevation(ai)));
//               else  
//                 _vertices_vw.push_back(Point3(this->get_point_x(ai), this->get_point_y(ai), curnc[sa]));
//               _vertices_vw.push_back(Point3(this->get_point_x(bi), this->get_point_y(bi), nnc[j]));
//               _vertices_vw.push_back(Point3(this->get_point_x(bi), this->get_point_y(bi), nnc[j + 1]));
//               Triangle t;
//               t.v0 = int(_vertices_vw.size()) - 3;
//               t.v1 = int(_vertices_vw.size()) - 2;
//               t.v2 = int(_vertices_vw.size()) - 1;
//               _triangles_vw.push_back(t);
//             }
//           }
//           if (curnc.size() > 0) {
//             for (int j = sa; j < ea; j++) {
//               if (nnc.size() == 0)
//                 _vertices_vw.push_back(Point3(this->get_point_x(bi), this->get_point_y(bi), this->get_point_elevation(bi)));
//               else
//                 _vertices_vw.push_back(Point3(this->get_point_x(bi), this->get_point_y(bi), nnc[eb]));
//               _vertices_vw.push_back(Point3(this->get_point_x(ai), this->get_point_y(ai), curnc[j]));
//               _vertices_vw.push_back(Point3(this->get_point_x(ai), this->get_point_y(ai), curnc[j + 1]));
//               Triangle t;
//               t.v0 = int(_vertices_vw.size()) - 3;
//               t.v1 = int(_vertices_vw.size()) - 1;
//               t.v2 = int(_vertices_vw.size()) - 2;
//               _triangles_vw.push_back(t);
//             }
//           }
//         }
//       }
//     }
//     ai++;
//   }
}
    

bool TopoFeature::has_segment(Point2& a, Point2& b) {
  double threshold = 0.001;
  int ringi, pi;
  if (this->has_point2(a, ringi, pi) == true) {
    Point2 tmp;
    tmp = this->get_next_point2_in_ring(ringi, pi);
    if (bg::distance(b, tmp) <= threshold)
      return true;
  }
  return false;
}


bool TopoFeature::has_point2(const Point2& p) {
  int a, b;
  return has_point2(p, a, b);
}


bool TopoFeature::has_point2(const Point2& p, int& ringi, int& pi) {
  double threshold = 0.001;
  Ring2 oring = bg::exterior_ring(*_p2);
  ringi = 0;
  for (int i = 0; i < oring.size(); i++) {
    if (bg::distance(p, oring[i]) <= threshold) {
      pi = i;
      return true;
    }
  }
  ringi++;
  auto irings = bg::interior_rings(*_p2);
  for (Ring2& iring: irings) {
    for (int i = 0; i < iring.size(); i++) {
      if (bg::distance(p, iring[i]) <= threshold) {
        pi = i;
        return true;
      }
    }
    ringi++;
  }
  return false;
}


Point2 TopoFeature::get_next_point2_in_ring(int ringi, int& pi) {
  Ring2 ring;
  if (ringi == 0) 
    ring = _p2->outer();
  else
    ring = _p2->inners()[ringi - 1];
  
  if (pi == (ring.size() - 1)) {
    pi = 0;
    return ring.front();
  }
  else {
    pi += 1;
    return ring[pi];
  }
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


int TopoFeature::get_vertex_elevation(int ringi, int pi) {
  return _p2z[ringi][pi];
}


void TopoFeature::set_vertex_elevation(int ringi, int pi, int z) {
  _p2z[ringi][pi] = z;  
}

//-- used to collect all LiDAR points linked to the polygon
//-- later all these values will be used to lift the polygon (and put values in _p2z)
bool TopoFeature::assign_elevation_to_vertex(double x, double y, double z, float radius) {
  Point2 p(x, y);
  int zcm = int(z * 100);
  int ringi = 0;
  Ring2 oring = bg::exterior_ring(*(_p2));
  for (int i = 0; i < oring.size(); i++) {
    if (bg::distance(p, oring[i]) <= radius)
      (_lidarelevs[ringi][i]).push_back(zcm);
  }
  ringi++;
  auto irings = bg::interior_rings(*(_p2));
  for (Ring2& iring: irings) {
    for (int i = 0; i < iring.size(); i++) {
      if (bg::distance(p, iring) <= radius) {
        (_lidarelevs[ringi][i]).push_back(zcm);
      }
    }
    ringi++;
  }
  return true;
}


void TopoFeature::lift_all_boundary_vertices_same_height(int height) {
  int ringi = 0;
  Ring2 oring = bg::exterior_ring(*(_p2));
  for (int i = 0; i < oring.size(); i++) 
    _p2z[ringi][i] = height;
  ringi++;
  auto irings = bg::interior_rings(*(_p2));
  for (Ring2& iring: irings) {
    for (int i = 0; i < iring.size(); i++) 
      _p2z[ringi][i] = height;
    ringi++;
  }
}


void TopoFeature::lift_each_boundary_vertices(float percentile) {
  //-- 1. assign value for each vertex based on percentile
  int ringi = 0;
  Ring2 oring = bg::exterior_ring(*(_p2));
  for (int i = 0; i < oring.size(); i++) {
    std::vector<int> &l = _lidarelevs[ringi][i];
    if (l.empty() == true)
      _p2z[ringi][i] = -9999;
    else {
      std::nth_element(l.begin(), l.begin() + (l.size() * percentile), l.end());
      _p2z[ringi][i] = l[l.size() * percentile];
    }
  }
  ringi++;
  auto irings = bg::interior_rings(*(_p2));
  for (Ring2& iring: irings) {
    for (int i = 0; i < iring.size(); i++) {
      std::vector<int> &l = _lidarelevs[ringi][i];
      if (l.empty() == true)
        _p2z[ringi][i] = -9999;
      else {
        std::nth_element(l.begin(), l.begin() + (l.size() * percentile), l.end());
        _p2z[ringi][i] = l[l.size() * percentile];
      }    
    }
    ringi++;
  }
  //-- 2. some vertices will have no values (no lidar point within tolerance thus)
  //--    assign them z value of next vertex in the ring
  //--    put 0 elevation if none? they will be stitched later on I hope/guess
  ringi = 0;
  oring = bg::exterior_ring(*(_p2));
  int pi;
  for (int i = 0; i < oring.size(); i++) {
    if (_p2z[ringi][i] == -9999) {
      pi = i;
      int j;
      for (j = 0; j < 1000; j++) {
        get_next_point2_in_ring(ringi, pi);
        if (_p2z[ringi][pi] != -9999)
          break;
      }
      if (j == 1000)
        _p2z[ringi][i] = 0;
      else
      _p2z[ringi][i] = _p2z[ringi][pi];
    } 
  }
  ringi++;
  irings = bg::interior_rings(*(_p2));
  for (Ring2& iring: irings) {
    for (int i = 0; i < iring.size(); i++) {
      if (_p2z[ringi][i] == -9999) {
        pi = i;
        int j;
        for (j = 0; j < 1000; j++) {
          get_next_point2_in_ring(ringi, pi);
          if (_p2z[ringi][pi] != -9999)
            break;
        }
        if (j == 1000)
          _p2z[ringi][i] = 0;
        else
        _p2z[ringi][i] = _p2z[ringi][pi];
      }    
    }
    ringi++;
  }
}  
  
//-------------------------------
//-------------------------------


Flat::Flat(char *wkt, std::string pid) 
: TopoFeature(wkt, pid) 
{}


int Flat::get_number_vertices() {
  // return int(2 * _vertices.size());
  return ( int(_vertices.size()) + int(_vertices_vw.size()) );
}

bool Flat::lift_percentile(float percentile) {
  int z = 0;
  if (_zvaluesinside.empty() == false) {
    std::nth_element(_zvaluesinside.begin(), _zvaluesinside.begin() + (_zvaluesinside.size() * percentile), _zvaluesinside.end());
    z = _zvaluesinside[_zvaluesinside.size() * percentile];
  }
  this->lift_all_boundary_vertices_same_height(z);
  _zvaluesinside.clear();
  _zvaluesinside.shrink_to_fit();
  return true;
}

int Flat::get_height() {
  return get_vertex_elevation(0, 0);
}

// std::string Flat::get_obj_v(int z_exaggeration) {
//   std::stringstream ss;
//   for (auto& v : _vertices)
//     ss << std::setprecision(3) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << (z_exaggeration > 0? (z_exaggeration * this->get_height_top()) : this->get_height_top()) << std::endl;
//   for (auto& v : _vertices)
//     ss << std::setprecision(3) << std::fixed << "v " << bg::get<0>(v) << " " << bg::get<1>(v) << " " << (z_exaggeration > 0? (z_exaggeration * this->get_height_base()) : this->get_height_base()) << std::endl;
//   return ss.str();
// }

// std::string Flat::get_obj_f(int offset, bool usemtl) {
//   std::stringstream ss;
//   //-- top surface
//   for (auto& t : _triangles)
//     ss << "f " << (t.v0 + 1 + offset) << " " << (t.v1 + 1 + offset) << " " << (t.v2 + 1 + offset) << std::endl;
  
// //-- extract segments
//   std::vector<Segment> allsegments;
//   for (auto& curt : _triangles) {
//     bool issegment = false;
//     for (auto& t : _triangles) {
//       if (triangle_contains_segment(t, curt.v1, curt.v0) == true) {
//         issegment = true;
//         break;
//       }
//     }
//     if (issegment == false) {
//       Segment s;
//       s.v0 = curt.v0;
//       s.v1 = curt.v1;
//       allsegments.push_back(s);
//     }
//     issegment = false;
//     for (auto& t : _triangles) {
//       if (triangle_contains_segment(t, curt.v2, curt.v1) == true) {
//         issegment = true;
//         break;
//       }
//     }
//     if (issegment == false) {
//       Segment s;
//       s.v0 = curt.v1;
//       s.v1 = curt.v2;
//       allsegments.push_back(s);
//     }
//     issegment = false;
//     for (auto& t : _triangles) {
//       if (triangle_contains_segment(t, curt.v0, curt.v2) == true) {
//         issegment = true;
//         break;
//       }
//     }
//     if (issegment == false) {
//       Segment s;
//       s.v0 = curt.v2;
//       s.v1 = curt.v0;
//       allsegments.push_back(s);
//     }
//   }
//   //-- side surfaces walls
//   for (auto& s : allsegments) {
//     ss << "f " << (s.v1 + 1 + offset) << " " << (s.v0 + 1 + offset) << " " << (s.v0 + 1 + offset + _vertices.size()) << std::endl;  
//     ss << "f " << (s.v0 + 1 + offset + _vertices.size()) << " " << (s.v1 + 1 + offset + _vertices.size()) << " " << (s.v1 + 1 + offset) << std::endl;  
//   }
//   return ss.str();
// }


bool Flat::add_elevation_point(double x, double y, double z, float radius, bool lastreturn) {
  Point2 p(x, y);
  int zcm = int(z * 100);
  //-- 1. assign to polygon if inside
  if (bg::within(p, *(_p2)) == true)
    _zvaluesinside.push_back(zcm);
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
  std::vector<int> tmp;
  for (int p = 0; p < passes; p++) {
    for (auto& r : _p2z) {
      tmp.resize(r.size());
      tmp.front() = int( (r[1] + r.back()) / 2);
      auto it = r.end();
      it -= 2;
      tmp.back() = int((r.front() + *it) / 2);
      for (int i = 1; i < (r.size() - 1); i++) 
        tmp[i] = int( (r[i - 1] + r[i + 1]) / 2);
    }
  }
}

// void Boundary3D::smooth_boundary(int passes) {
//   for (int p = 0; p < passes; p++) {
//     int ringi = 0;
//     Ring2 oring = bg::exterior_ring(*(_p2));
//     std::vector<int> elevs(bg::num_points(oring));
//     smooth_ring(_p2z[ringi], elevs);
//     for (int i = 0; i < oring.size(); i++) 
//       _p2z[ringi][i] = elevs[i];
//     ringi++;
//     auto irings = bg::interior_rings(*_p2);
//     for (Ring2& iring: irings) {
//       elevs.resize(bg::num_points(iring));
//       smooth_ring(_p2z[ringi], elevs);
//       for (int i = 0; i < iring.size(); i++) 
//         _p2z[ringi][i] = elevs[i];
//       ringi++;
//     }
//   }
// }

// void Boundary3D::smooth_ring(const std::vector<int> &r, std::vector<int> &elevs) {
//   elevs.front() = (bg::get<2>(r[1]) + bg::get<2>(r.back())) / 2;
//   auto it = r.end();
//   it -= 2;
//   elevs.back() = (bg::get<2>(r.front()) + bg::get<2>(*it)) / 2;
//   for (int i = 1; i < (r.size() - 1); i++) 
//     elevs[i] = (bg::get<2>(r[i - 1]) + bg::get<2>(r[i + 1])) / 2;
// }



//-------------------------------
//-------------------------------

TIN::TIN(char *wkt, std::string pid, int simplification) 
: TopoFeature(wkt, pid) 
{
  _simplification = simplification;
}


bool TIN::buildCDT() {
  getCDT(_p2, _p2z, _vertices, _triangles, _lidarpts);
  return true;
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



