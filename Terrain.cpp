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

#include "Terrain.h"
#include <algorithm>


Terrain::Terrain (char *wkt, std::string pid, int simplification) : TIN(wkt, pid, simplification)
{}

std::string Terrain::get_obj_f(int offset, bool usemtl) {
  std::stringstream ss;
  if (usemtl == true)
    ss << "usemtl Terrain" << std::endl;
  ss << TopoFeature::get_obj_f(offset, usemtl);
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
  //-- lift vertices to their median of lidar points
  TopoFeature::lift_vertices_boundary(0.5);
  return true;
}

bool Terrain::buildCDT() {
  getCDT(&_p3, _vertices, _triangles, _segments, _lidarpts);

  int i = 0;
  for (auto& curnc : _nc) {
    if (curnc.empty() == false) {
      curnc.push_back(this->get_point_elevation(i));
      std::sort(curnc.begin(), curnc.end());
    }
    i++;
  }
  //-- add those evil vertical walls
  i = 0;
  int ni;
  for (auto& curnc : _nc) {
    this->get_next_point2_in_ring(i, ni); //-- index of next in ring
    std::vector<float> nnc = _nc[ni];

    if (nnc.size() > 0) {
      for (int j = 0; j < (nnc.size() - 1); j++) {
        _vertices.push_back(Point3(this->get_point_x(i), this->get_point_y(i), this->get_point_elevation(i)));
        _vertices.push_back(Point3(this->get_point_x(ni), this->get_point_y(ni), nnc[j]));
        _vertices.push_back(Point3(this->get_point_x(ni), this->get_point_y(ni), nnc[j + 1]));
        Triangle t;
        t.v0 = int(_vertices.size()) - 3;
        t.v1 = int(_vertices.size()) - 2;
        t.v2 = int(_vertices.size()) - 1;
        _triangles.push_back(t);
      }
    }

    if (curnc.size() > 0) {
      for (int j = 0; j < (curnc.size() - 1); j++) {
        if (nnc.size() == 0)
          _vertices.push_back(Point3(this->get_point_x(ni), this->get_point_y(ni), this->get_point_elevation(ni)));
        else
          _vertices.push_back(Point3(this->get_point_x(ni), this->get_point_y(ni), nnc.front()));
        _vertices.push_back(Point3(this->get_point_x(i), this->get_point_y(i), curnc[j]));
        _vertices.push_back(Point3(this->get_point_x(i), this->get_point_y(i), curnc[j + 1]));
        Triangle t;
        t.v0 = int(_vertices.size()) - 3;
        t.v1 = int(_vertices.size()) - 2;
        t.v2 = int(_vertices.size()) - 1;
        _triangles.push_back(t);
      }
    }
    i++;
  }
  return true;
}
