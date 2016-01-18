//
//  Terrain.cpp
//
//  Created by Hugo Ledoux on 19/10/15.
//  Copyright © 2015 Hugo Ledoux. All rights reserved.
//

#include "Terrain.h"


Terrain::Terrain (Polygon2* p, std::string pid, int simplification) : TIN(p, pid, simplification)
{}

std::string Terrain::get_obj_f(int offset) {
  std::stringstream ss;
  ss << "usemtl Terrain" << std::endl;
  ss << TopoFeature::get_obj_f(offset);
  return ss.str();
}

TopoClass Terrain::get_class() {
  return TERRAIN;
}

bool Terrain::is_hard() {
  return false;
}


std::string Terrain::get_citygml() {
  return "<EMPTY/>";
}

bool Terrain::lift() {
  std::stringstream ss;
  ss << bg::wkt(*(_p2));
  bg::read_wkt(ss.str(), _p3);
  //-- lift vertices to their median of lidar points
  TopoFeature::lift_vertices_boundary(0.5);
  return true;
}

bool Terrain::buildCDT() {
  getCDT(&_p3, _vertices, _triangles, _segments, _lidarpts);

  // std::cout << "llllllll" << std::endl;
  // std::cout << std::setprecision(3) << std::fixed << bg::wkt(*(_p2)) << std::endl;
  // std::cout << std::setprecision(3) << std::fixed << this->get_point_x(0) << std::endl;
  // // std::cout << std::setprecision(3) << std::fixed << bg::wkt(_p3) << std::endl;
  // std::cout << "llllllll" << std::endl;

  //-- add those evil vertical walls
  int i = 0;
  int ni;
  for (auto& curnc : _nc) {
    this->get_next_point2_in_ring(i, ni); //-- index of next in ring
    std::vector<float> nnc = _nc[ni];

    if ( (curnc.size() == 0) && (nnc.size() == 0))
      std::clog << "both nc empty" << std::endl;
    else {
      std::clog << "vertical wall to add." << std::endl;
      for (auto& v : nnc) {
        _vertices.push_back(Point3(this->get_point_x(i), this->get_point_y(i), this->get_point_elevation(i)));
        Triangle t;
        // t.v0 = out.trianglelist[i * out.numberofcorners + 0];
        // t.v1 = out.trianglelist[i * out.numberofcorners + 1];
        // t.v2 = out.trianglelist[i * out.numberofcorners + 2];
        // triangles.push_back(t);
      }
    }
    

    if (curnc.size() > 0) {
      std::clog << "**nc** " << this->get_point_elevation(i) << std::endl;
      for (auto& n : curnc) {
        std::clog << "|" << n;
      }
      std::clog << "( " << _nc[ni].size() << " ) ";
      std::clog << std::endl;
    }
    i++;
  }

  return true;
}
