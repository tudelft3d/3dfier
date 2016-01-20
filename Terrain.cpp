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


Terrain::Terrain (char *wkt, std::string pid, int simplification) : TIN(wkt, pid, simplification)
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
  // int i = 0;
  // int ni;
  // for (auto& curnc : _nc) {
  //   this->get_next_point2_in_ring(i, ni); //-- index of next in ring
  //   std::vector<float> nnc = _nc[ni];

  //   if ( (curnc.size() == 0) && (nnc.size() == 0))
  //     std::clog << "both nc empty" << std::endl;
  //   else {
  //     std::clog << "vertical wall to add." << std::endl;
  //     for (auto& v : nnc) {
  //       _vertices.push_back(Point3(this->get_point_x(i), this->get_point_y(i), this->get_point_elevation(i)));
  //       Triangle t;
  //       // t.v0 = out.trianglelist[i * out.numberofcorners + 0];
  //       // t.v1 = out.trianglelist[i * out.numberofcorners + 1];
  //       // t.v2 = out.trianglelist[i * out.numberofcorners + 2];
  //       // triangles.push_back(t);
  //     }
  //   }
    

  //   if (curnc.size() > 0) {
  //     std::clog << "**nc** " << this->get_point_elevation(i) << std::endl;
  //     for (auto& n : curnc) {
  //       std::clog << "|" << n;
  //     }
  //     std::clog << "( " << _nc[ni].size() << " ) ";
  //     std::clog << std::endl;
  //   }
  //   i++;
  // }

  return true;
}
