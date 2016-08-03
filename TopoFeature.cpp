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
  _bVerticalWalls = false;
  _p2 = new Polygon2();
  bg::read_wkt(wkt, *(_p2));
  
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
  getCDT(_p2, _p2z, _vertices, _vertices_map, _triangles);
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

std::string TopoFeature::get_obj_v(std::vector<Point3>::size_type &idx, std::unordered_map<std::string, std::vector<Point3>::size_type> &map, int z_exaggeration) {
  std::string obj;
  for (auto& v : _vertices) {
    std::string key = gen_key_bucket(&v);
    if (map.find(key) == map.end()) {
      map[key] = idx;
      idx++;
      char* buf = new char[200];
      std::sprintf(buf, "v %.3f %.3f %.3f\n", bg::get<0>(v),  bg::get<1>(v), 
          (z_exaggeration > 0 ? (z_exaggeration * bg::get<2>(v)) : bg::get<2>(v)));
      obj += std::string(buf);
    }
  }
  //for (auto& v : _vertices_vw) {
  //  char* buf = new char[200];
  //  std::sprintf(buf, "v %.3f %.3f %.3f\n", bg::get<0>(v), bg::get<1>(v),
  //      (z_exaggeration > 0 ? (z_exaggeration * bg::get<2>(v)) : bg::get<2>(v)));
  //  obj += std::string(buf);
  //}
  return obj;
}

std::string TopoFeature::get_obj_f(std::unordered_map<std::string, std::vector<Point3>::size_type> &map, bool usemtl) {
  std::string obj;
  for (auto& t : _triangles) {
    char* buf = new char[200];
    //std::sprintf(buf, "f %i %i %i\n", t.v0 + 1 + offset, t.v1 + 1 + offset, t.v2 + 1 + offset);
    std::sprintf(buf, "f %lu %lu %lu\n", map[t.v0], map[t.v1], map[t.v2]);
    obj += std::string(buf);
  }

  if (usemtl == true && _triangles_vw.size() > 0)
    obj += "usemtl VerticalWalls\n";

  for (auto& t : _triangles_vw) {
    char* buf = new char[200];
    //std::sprintf(buf, "f %lu %lu %lu\n", t.v0 + 1 + offset + k, t.v1 + 1 + offset + k, t.v2 + 1 + offset + k);
    std::sprintf(buf, "f %lu %lu %lu\n", map[t.v0], map[t.v1], map[t.v2]);

    obj += std::string(buf);
  }
  return obj;
}

std::string TopoFeature::get_wkt() {
  std::string wkt;
  wkt = "MULTIPOLYGONZ (";

  for (auto& t : _triangles) {
    char* buf = new char[200];
    Point3 p1 = _vertices[_vertices_map[t.v0]];
    Point3 p2 = _vertices[_vertices_map[t.v1]];
    Point3 p3 = _vertices[_vertices_map[t.v2]];
    std::sprintf(buf, "((%.3f %.3f %.3f,%.3f %.3f %.3f,%.3f %.3f %.3f,%.3f %.3f %.3f)),",
      p1.get<0>(), p1.get<1>(), p1.get<2>(),
      p2.get<0>(), p2.get<1>(), p2.get<2>(),
      p3.get<0>(), p3.get<1>(), p3.get<2>(),
      p1.get<0>(), p1.get<1>(), p1.get<2>());
    wkt += std::string(buf);
  }

  for (auto& t : _triangles_vw) {
    char* buf = new char[200];
    Point3 p1 = _vertices[_vertices_map[t.v0]];
    Point3 p2 = _vertices[_vertices_map[t.v1]];
    Point3 p3 = _vertices[_vertices_map[t.v2]];
    std::sprintf(buf, "((%.3f %.3f %.3f,%.3f %.3f %.3f,%.3f %.3f %.3f,%.3f %.3f %.3f)),",
      p1.get<0>(), p1.get<1>(), p1.get<2>(),
      p2.get<0>(), p2.get<1>(), p2.get<2>(),
      p3.get<0>(), p3.get<1>(), p3.get<2>(),
      p1.get<0>(), p1.get<1>(), p1.get<2>());
    wkt += std::string(buf);
  }
  wkt.pop_back();
  wkt += ")";
  return wkt;
}

bool TopoFeature::get_shape_features(OGRLayer* layer, std::string className) {
  OGRFeatureDefn *featureDefn = layer->GetLayerDefn();
  OGRFeature *feature = OGRFeature::CreateFeature(featureDefn);
  OGRMultiPolygon multipolygon = OGRMultiPolygon();
  Point3 p;

  //-- add all triangles to the layer
  for (auto& t : _triangles) {
    OGRPolygon polygon = OGRPolygon();
    OGRLinearRing ring = OGRLinearRing();
    
    p = _vertices[_vertices_map[t.v0]];
    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
    p = _vertices[_vertices_map[t.v1]];
    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
    p = _vertices[_vertices_map[t.v2]];
    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());

    ring.closeRings();
    polygon.addRing(&ring);
    multipolygon.addGeometry(&polygon);
  }

  //-- add all vertical wall triangles to the layer
  for (auto& t : _triangles_vw) {
    OGRPolygon polygon = OGRPolygon();
    OGRLinearRing ring = OGRLinearRing();

    p = _vertices[_vertices_map[t.v0]];
    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
    p = _vertices[_vertices_map[t.v1]];
    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
    p = _vertices[_vertices_map[t.v2]];
    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());

    ring.closeRings();
    polygon.addRing(&ring);
    multipolygon.addGeometry(&polygon);
  }

  feature->SetGeometry(&multipolygon);
  feature->SetField("Id", this->get_id().c_str());
  feature->SetField("Class", className.c_str());

  if (layer->CreateFeature(feature) != OGRERR_NONE)
  {
    std::cerr << "Failed to create feature " << this->get_id() << " in shapefile." << std::endl;
    return false;
  }
  OGRFeature::DestroyFeature(feature);
  return true;
}

void TopoFeature::fix_bowtie(std::vector<TopoFeature*> lsAdj) {
  //-- gather all rings
  std::vector<Ring2> therings;
  therings.push_back(bg::exterior_ring(*(_p2)));
  for (auto& iring : bg::interior_rings(*(_p2)))
    therings.push_back(iring);

  //-- process each vertex of the polygon separately
  std::vector<int> anc, bnc;
  Point2 a, b;
  TopoFeature* fadj;
  int ringi = -1;
  for (auto& ring : therings) {
    ringi++;
    for (int ai = 0; ai < ring.size(); ai++) {
      //-- Point a
      a = ring[ai];
      //-- find Point b
      int bi;
      if (ai == (ring.size() - 1)) {
        b = ring.front();
        bi = 0;
      }
      else {
        b = ring[ai + 1];
        bi = ai + 1;
      }
      //-- find the adjacent polygon to segment ab (fadj)
      fadj = nullptr;
      int adj_a_ringi = 0;
      int adj_a_pi = 0;
      int adj_b_ringi = 0;
      int adj_b_pi = 0;
      for (auto& adj : lsAdj) {
        if (adj->has_segment(b, a, adj_b_ringi, adj_b_pi, adj_a_ringi, adj_a_pi) == true) {
          // if (adj->has_segment(b, a) == true) {
          fadj = adj;
          break;
        }
      }
      if (fadj == nullptr)
        continue;
      //-- check height differences: f > fadj for *both* Points a and b
      int az = this->get_vertex_elevation(ringi, ai);
      int bz = this->get_vertex_elevation(ringi, bi);
      int fadj_az = fadj->get_vertex_elevation(adj_a_ringi, adj_a_pi);
      int fadj_bz = fadj->get_vertex_elevation(adj_b_ringi, adj_b_pi);

      //-- Fix bow-ties
      if (((az > fadj_az) && (bz < fadj_bz)) || ((az < fadj_az) && (bz > fadj_bz))) {
        //std::clog << "BOWTIE:" << this->get_id() << " & " << fadj->get_id() << std::endl;
        //std::clog << this->get_class() << " & " << fadj->get_class() << std::endl;
        if (this->is_hard() && fadj->is_hard() == false) {
          //- this is hard, snap the smallest height of the soft feature to this
          if (abs(az - fadj_az) < abs(bz - fadj_bz)) {
            fadj->set_vertex_elevation(adj_a_ringi, adj_a_pi, az);
          }
          else {
            fadj->set_vertex_elevation(adj_b_ringi, adj_b_pi, bz);
          }
        }
        else if (this->is_hard() == false && fadj->is_hard()) {
          //- this is soft, snap the smallest height to the hard feature
          if (abs(az - fadj_az) < abs(bz - fadj_bz)) {
            this->set_vertex_elevation(ringi, ai, fadj_az);
          }
          else {
            this->set_vertex_elevation(ringi, bi, fadj_bz);
          }
        }
        else {
          if (abs(az - fadj_az) < abs(bz - fadj_bz)) {
            //- snap a to lowest
            if (az < fadj_az) {
              fadj->set_vertex_elevation(adj_a_ringi, adj_a_pi, az);
            }
            else
            {
              this->set_vertex_elevation(ringi, ai, fadj_az);
            }
          }
          else {
            //- snap b to lowest
            if (bz < fadj_bz) {
              fadj->set_vertex_elevation(adj_b_ringi, adj_b_pi, bz);
            }
            else {
              this->set_vertex_elevation(ringi, bi, fadj_bz);
            }
          }
        }
      }
    }
  }
}


void TopoFeature::construct_vertical_walls(std::vector<TopoFeature*> lsAdj, std::unordered_map<std::string, std::vector<int>> nc) {
  if (this->has_vertical_walls() == false)
    return;

  //-- gather all rings
  std::vector<Ring2> therings;
  therings.push_back(bg::exterior_ring(*(_p2)));
  for (auto& iring: bg::interior_rings(*(_p2))) 
    therings.push_back(iring);

  //-- process each vertex of the polygon separately
  std::vector<int> anc, bnc;
  Point2 a, b;
  TopoFeature* fadj;
  int ringi = -1;
  for (auto& ring : therings) {
    ringi++;
    for (int ai = 0; ai < ring.size(); ai++) {
      //-- Point a
      a = ring[ai];
      //-- find Point b
      int bi;
      if (ai == (ring.size() - 1)) {
        b = ring.front();
        bi = 0;
      }
      else {
        b = ring[ai + 1];
        bi = ai + 1;
      }
      //-- find the adjacent polygon to segment ab (fadj)
      fadj = nullptr;
      int adj_a_ringi = 0;
      int adj_a_pi = 0;
      int adj_b_ringi = 0;
      int adj_b_pi = 0;
      for (auto& adj : lsAdj) {
        if (adj->has_segment(b, a, adj_b_ringi, adj_b_pi, adj_a_ringi, adj_a_pi) == true) {
          // if (adj->has_segment(b, a) == true) {
          fadj = adj;
          break;
        }
      }
      if (fadj == nullptr)
        continue;
      //-- check height differences: f > fadj for *both* Points a and b
      int az = this->get_vertex_elevation(ringi, ai);
      int bz = this->get_vertex_elevation(ringi, bi);
      int fadj_az = fadj->get_vertex_elevation(adj_a_ringi, adj_a_pi);
      int fadj_bz = fadj->get_vertex_elevation(adj_b_ringi, adj_b_pi);

      if ((az < fadj_az) || (bz < fadj_bz))
        continue;
      if ((az == fadj_az) && (bz == fadj_bz))
        continue;

      try {
        anc = nc.at(gen_key_bucket(&a));
      }
      catch (const std::out_of_range& oor) {
        std::cerr << "Id: " << this->get_id() << " - Class: " << this->get_class() << " - Out of Range error anc - " << gen_key_bucket(&a) << std::endl;
      }
      try {
        bnc = nc.at(gen_key_bucket(&b));
      }
      catch (const std::out_of_range& oor) {
        std::cerr << "Id: " << this->get_id() << " - Class: " << this->get_class() << " - Out of Range error bnc - " << gen_key_bucket(&b) << std::endl;
      }

      //std::clog << "az: " << az << std::endl;
      //std::clog << "bz: " << bz << std::endl;
      //std::clog << "fadj_az: " << fadj_az << std::endl;
      //std::clog << "fadj_bz: " << fadj_bz << std::endl;

	    //-- find the height of the vertex in the node column
      std::vector<int>::iterator sait, eait, sbit, ebit;
      sait = std::find(anc.begin(), anc.end(), fadj_az);
      eait = std::find(anc.begin(), anc.end(), az);
      sbit = std::find(bnc.begin(), bnc.end(), fadj_bz);
      ebit = std::find(bnc.begin(), bnc.end(), bz);

      //int wrongit = 0;
      //if (sait == anc.end()) {
      //  std::clog << "WRONG ITERATOR sait" << std::endl;
      //  wrongit++;
      //}
      //else
      //  std::clog << *sait << std::endl;
      //if (eait == anc.end()) {
      //  std::clog << "WRONG ITERATOR eait" << std::endl;
      //  wrongit++;
      //}
      //else
      //  std::clog << *eait << std::endl;
      //if (sbit == bnc.end()) {
      //  std::clog << "WRONG ITERATOR sbit" << std::endl;
      //  wrongit++;
      //}
      //else
      //  std::clog << *sbit << std::endl;
      //if (ebit == bnc.end()) {
      //  std::clog << "WRONG ITERATOR ebit" << std::endl;
      //  wrongit++;
      //}
      //else
      //  std::clog << *ebit << std::endl;

      //if (wrongit == 3) { //check if there is an uneven amount of wrong iterators
      //  std::clog << "WRONG AMOUNT OF ITERATORS" << std::endl;
      //}
      //if (wrongit != 4 && eait == anc.end() && ebit == bnc.end()) {
      //  std::cerr << "BOTH ITERATORS END" << std::endl;
      //}

      //-- iterate to triangulate
      while (sbit != ebit && sbit != bnc.end() && (sbit+1) != bnc.end()) {
        //if (anc.size() == 0 || sait == anc.end())
        //  _vertices_vw.push_back(Point3(bg::get<0>(a), bg::get<1>(a), float(az) / 100));
        //else
        //  _vertices_vw.push_back(Point3(bg::get<0>(a), bg::get<1>(a), float(*sait) / 100));
        //_vertices_vw.push_back(Point3(bg::get<0>(b), bg::get<1>(b), float(*sbit) / 100));
        Triangle t;
        Point3 p;
        std::string key;
        if (anc.size() == 0 || sait == anc.end()) {
          p = Point3(bg::get<0>(a), bg::get<1>(a), float(az) / 100);
        }
        else {
          p = Point3(bg::get<0>(a), bg::get<1>(a), float(*sait) / 100);
        }
        key = gen_key_bucket(&p);
        if (_vertices_map.find(key) == _vertices_map.end()) {
          _vertices.push_back(p);
          _vertices_map[key] = _vertices.size() - 1;
        }
        t.v1 = key;

        p = Point3(bg::get<0>(b), bg::get<1>(b), float(*sbit) / 100);
        key = gen_key_bucket(&p);
        if (_vertices_map.find(key) == _vertices_map.end()) {
          _vertices.push_back(p);
          _vertices_map[key] = _vertices.size() - 1;
        }
        t.v0 = key;
        
        sbit++;
        p = Point3(bg::get<0>(b), bg::get<1>(b), float(*sbit) / 100);
        key = gen_key_bucket(&p);
        if (_vertices_map.find(key) == _vertices_map.end()) {
          _vertices.push_back(p);
          _vertices_map[key] = _vertices.size() - 1;
        }
        t.v2 = key;

        if (t.v0 != t.v1 && t.v0 != t.v2 && t.v1 != t.v2) {
          _triangles_vw.push_back(t);
        }
      }
      while (sait != eait && sait != anc.end() && (sait + 1) != anc.end()) {
        //if (bnc.size() == 0 || ebit == bnc.end())
        //  _vertices_vw.push_back(Point3(bg::get<0>(b), bg::get<1>(b), float(bz) / 100));
        //else
        //  _vertices_vw.push_back(Point3(bg::get<0>(b), bg::get<1>(b), float(*ebit) / 100));
        //_vertices_vw.push_back(Point3(bg::get<0>(a), bg::get<1>(a), float(*sait) / 100));
        Triangle t;
        Point3 p;
        std::string key;
        if (bnc.size() == 0 || ebit == bnc.end()) {
          p = Point3(bg::get<0>(b), bg::get<1>(b), float(bz) / 100);
        }
        else {
          p = Point3(bg::get<0>(b), bg::get<1>(b), float(*ebit) / 100);
        }
        key = gen_key_bucket(&p);
        if (_vertices_map.find(key) == _vertices_map.end()) {
          _vertices.push_back(p);
          _vertices_map[key] = _vertices.size() - 1;
        }
        t.v0 = key;

        p = Point3(bg::get<0>(a), bg::get<1>(a), float(*sait) / 100);
        key = gen_key_bucket(&p);
        if (_vertices_map.find(key) == _vertices_map.end()) {
          _vertices.push_back(p);
          _vertices_map[key] = _vertices.size() - 1;
        }
        t.v1 = key;

        sait++;
        p = Point3(bg::get<0>(a), bg::get<1>(a), float(*sait) / 100);
        key = gen_key_bucket(&p);
        if (_vertices_map.find(key) == _vertices_map.end()) {
          _vertices.push_back(p);
          _vertices_map[key] = _vertices.size() - 1;
        }
        t.v2 = key;

        if (t.v0 != t.v1 && t.v0 != t.v2 && t.v1 != t.v2) {
          _triangles_vw.push_back(t);
        }
      }
    }
  } 
}


bool TopoFeature::has_segment(Point2& a, Point2& b, int& aringi, int& api, int& bringi, int& bpi) {
  double threshold = 0.001;
  std::vector<int> ringis, pis;
  Point2 tmp;
  int nextpi;
  if (this->has_point2_(a, ringis, pis) == true) {
    for (int k = 0; k < ringis.size(); k++) {
      nextpi = pis[k];
      tmp = this->get_next_point2_in_ring(ringis[k], nextpi);
      if (bg::distance(b, tmp) <= threshold) {
        aringi = ringis[k];
        api = pis[k];
        bringi = ringis[k];
        bpi = nextpi;
        return true;
      }
    }

  }
  return false;
}


float TopoFeature::get_distance_to_boundaries(Point2& p) {
  //-- collect the rings of the polygon
  std::vector<Ring2> therings;
  therings.push_back(bg::exterior_ring(*(_p2)));
  for (auto& iring: bg::interior_rings(*(_p2)))
      therings.push_back(iring);
  //-- process each vertex of the polygon separately
  Point2 a, b;
  Segment2 s;
  int ringi = -1;
  double dmin = 99999;
  for (auto& ring : therings) {
    ringi++;
    for (int ai = 0; ai < ring.size(); ai++) {
      a = ring[ai];
      if (ai == (ring.size() - 1))
          b = ring.front();
      else
          b = ring[ai + 1];
      //-- calculate distance
      bg::set<0, 0>(s, bg::get<0>(a));
      bg::set<0, 1>(s, bg::get<1>(a));
      bg::set<1, 0>(s, bg::get<0>(b));
      bg::set<1, 1>(s, bg::get<1>(b));
      float d = boost::geometry::distance(p, s);
      if (d < dmin)
          dmin = d;
    }
  }
  return dmin;
}


bool TopoFeature::has_segment(Point2& a, Point2& b) {
  double threshold = 0.001;
  std::vector<int> ringis, pis;
  if (this->has_point2_(a, ringis, pis) == true) {
    Point2 tmp;
    tmp = this->get_next_point2_in_ring(ringis[0], pis[0]);
    if (bg::distance(b, tmp) <= threshold)
      return true;
  }
  return false;
}


bool TopoFeature::has_point2(const Point2& p) {
  std::vector<int> a, b;
  return has_point2_(p, a, b);
}


bool TopoFeature::has_point2_(const Point2& p, std::vector<int>& ringis, std::vector<int>& pis) {
  double threshold = 0.001;
  Ring2 oring = bg::exterior_ring(*_p2);
  int ringi = 0;
  bool re = false;
  for (int i = 0; i < oring.size(); i++) {
    if (bg::distance(p, oring[i]) <= threshold) {
      ringis.push_back(ringi);
      pis.push_back(i);
      re = true;
      break;
    }
  }
  ringi++;
  auto irings = bg::interior_rings(*_p2);
  for (Ring2& iring: irings) {
    for (int i = 0; i < iring.size(); i++) {
      if (bg::distance(p, iring[i]) <= threshold) {
        ringis.push_back(ringi);
        pis.push_back(i);
        re = true;
        break;
      }
    }
    ringi++;
  }
  return re;
}

Point2 TopoFeature::get_point2(int ringi, int pi) {
  Ring2 ring;
  if (ringi == 0) 
    ring = _p2->outer();
  else
    ring = _p2->inners()[ringi - 1];
  return ring[pi];
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


bool TopoFeature::has_vertical_walls() {
  return _bVerticalWalls;
}

void TopoFeature::add_vertical_wall() {
  _bVerticalWalls = true;
}


int TopoFeature::get_vertex_elevation(int ringi, int pi) {
  return _p2z[ringi][pi];
}


int TopoFeature::get_vertex_elevation(Point2& p) {
  std::vector<int> ringis, pis;
  has_point2_(p, ringis, pis);
  return _p2z[ringis[0]][pis[0]];
}


void TopoFeature::set_vertex_elevation(int ringi, int pi, int z) {
  _p2z[ringi][pi] = z;  
}

//-- used to collect all LiDAR points linked to the polygon
//-- later all these values are used to lift the polygon (and put values in _p2z)
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
  //return ( int(_vertices.size()) + int(_vertices_vw.size()) );
  return int(_vertices.size());
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


bool Flat::add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  int zcm = int(z * 100);
  //-- 1. assign to polygon since within the threshold value (buffering of polygon)
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
  //return ( int(_vertices.size()) + int(_vertices_vw.size()) );
  return int(_vertices.size());
}


bool Boundary3D::add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn) {
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
  getCDT(_p2, _p2z, _vertices, _vertices_map, _triangles, _lidarpts);
  return true;
}


int TIN::get_number_vertices() {
  //return ( int(_vertices.size()) + int(_vertices_vw.size()) );
  return int(_vertices.size());
}


// bool TIN::add_elevation_point(double x, double y, double z, float radius, bool lastreturn) {
//   bool toadd = false;
//   float distance = 4.0;
//   if (_simplification <= 1)
//     toadd = true;
//   else {
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::uniform_int_distribution<int> dis(1, _simplification);
//     if (dis(gen) == 1)
//       toadd = true;
//   }
//   if (toadd == true) {
//     Point2 p(x, y);
//     if ( (bg::within(p, *(_p2)) == true) && (this->get_distance_to_boundaries(p) > (radius * 1.5)) ) 
//       _lidarpts.push_back(Point3(x, y, z));
//   }
//   if (lastreturn == true)
//     assign_elevation_to_vertex(x, y, z, radius);
//   return toadd;
// }




