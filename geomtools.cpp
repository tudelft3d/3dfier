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

#include "geomtools.h"



//-- TODO: cheap code that randomly generates points if centroid is not inside.
void get_point_inside(Ring2& ring, Point2& p) {
  bg::centroid(ring, p);
  if (bg::within(p, ring) == false) {
    Box2 bbox;
    bg::envelope(ring, bbox);
    Point2 genp;
    std::default_random_engine re;
    while (true) {
      std::uniform_real_distribution<double> unifx(bg::get<0>(bbox.min_corner()), bg::get<0>(bbox.max_corner()));
      std::uniform_real_distribution<double> unify(bg::get<1>(bbox.min_corner()), bg::get<1>(bbox.max_corner()));
      bg::set<0>(genp, unifx(re));
      bg::set<1>(genp, unify(re));
      if (bg::within(genp, ring) == true)
        break;
    }
    bg::set<0>(p, bg::get<0>(genp));
    bg::set<1>(p, bg::get<1>(genp));
  }
}

bool triangle_contains_segment(Triangle t, int a, int b) {
  if ( (t.v0 == a) && (t.v1 == b) )
    return true;
  if ( (t.v1 == a) && (t.v2 == b) )
    return true;
  if ( (t.v2 == a) && (t.v0 == b) )
    return true;
  return false;
}

// bool getCDT(const Polygon3* pgn, 
//             std::vector<Point3> &vertices, 
//             std::vector<Triangle> &triangles, 
//             const std::vector<Point3> &lidarpts) {
bool getCDT(const Polygon2* pgn,
            const std::vector< std::vector<int> > &z, 
            std::vector<Point3> &vertices, 
            std::vector<Triangle> &triangles, 
            const std::vector<Point3> &lidarpts) {
  Ring2 oring = bg::exterior_ring(*pgn);
  auto irings = bg::interior_rings(*pgn);
  
  struct triangulateio in, out;
  in.numberofpointattributes = 1;
  in.numberofpoints = int(bg::num_points(*pgn));
  in.numberofpoints += lidarpts.size();
  in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
  in.pointattributelist = (REAL *) malloc(in.numberofpoints * in.numberofpointattributes * sizeof(REAL));
  int counter = 0;
  //-- oring
  int ringi = 0;
  for (int i = 0; i < oring.size(); i++) {
    in.pointlist[counter++]  = bg::get<0>(oring[i]);
    in.pointlist[counter++]  = bg::get<1>(oring[i]);
//    std::cout << float(z[ringi][i]) / 100.0 << std::endl;
    in.pointattributelist[i] = (float(z[ringi][i]) / 100.0);
    // std::cout << std::setprecision(3) << std::fixed << bg::get<0>(oring[i]) << " | " << bg::get<1>(oring[i]) << " | " << bg::get<2>(oring[i]) << std::endl;
  }
  ringi++;
  //-- irings
  if (irings.size() == 0) {
    in.numberofholes = 0;
  }
  else {
    in.numberofholes = int(irings.size());
    in.holelist = (REAL *) malloc(in.numberofholes * 2 * sizeof(REAL));
    int holecount = 0;
    for (auto iring : irings) {
      for (int i = 0; i < iring.size(); i++) {
       in.pointlist[counter++] = bg::get<0>(iring[i]);
       in.pointlist[counter++] = bg::get<1>(iring[i]);
       in.pointattributelist[(counter / 2) - 1] = (float(z[ringi][i]) / 100.0);
      }
      ringi++;
      Point2 pinside;
      get_point_inside(iring, pinside);
      // std::clog << "pinside: " << bg::wkt(pinside) << std::endl;
      in.holelist[holecount++] = bg::get<0>(pinside);
      in.holelist[holecount++] = bg::get<1>(pinside);
//      in.pointattibutelist[(counter / 2) - 1] = 0.0;
    }
  }
  //-- add the lidar points to the CDT, if any
  for (auto &pt : lidarpts) {
    in.pointlist[counter++] = bg::get<0>(pt);
    in.pointlist[counter++] = bg::get<1>(pt); 
    in.pointattributelist[(counter / 2) - 1] = bg::get<2>(pt);
  }
  in.pointmarkerlist = NULL;
  //-- segments to store too
  in.numberofsegments = in.numberofpoints - int(lidarpts.size());
  in.numberofregions = 0;
  in.segmentlist = (int *) malloc(in.numberofsegments * 2 * sizeof(int));
  counter = 0;
  //-- oring
  int i;
  for (i = 0; i < (oring.size() - 1); i++) {
    in.segmentlist[counter++] = i;
    in.segmentlist[counter++] = (i + 1);
  }
  in.segmentlist[counter++] = i;
  in.segmentlist[counter++] = 0;
  //-- irings
  int start = int(oring.size());
  for (auto iring : irings) {
    int i;
    for (i = 0; i < (iring.size() - 1); i++) {
      in.segmentlist[counter++] = (start + i);
      in.segmentlist[counter++] = (start + i + 1);
    }
    in.segmentlist[counter++] = (start + i);
    in.segmentlist[counter++] = (start);
    start += iring.size();
  }
  in.segmentmarkerlist = NULL;

  //-- output from Triangle
  out.pointlist = (REAL *) NULL;
  out.pointattributelist = (REAL *) NULL;
  out.pointmarkerlist = (int *) NULL;
  out.trianglelist = (int *) NULL;
  out.triangleattributelist = (REAL *) NULL;
  if (in.numberofholes > 0)
    out.holelist = (REAL *) NULL;
  // out.regionlist = (REAL *) NULL;
  out.neighborlist = (int *) NULL;
  out.segmentlist = (int *) NULL;
  out.segmentmarkerlist = (int *) NULL;
  // out.edgelist = (int *) NULL;
  // out.edgemarkerlist = (int *) NULL;

  //-- call Triangle
  triangulate((char *)"pzQ", &in, &out, NULL);
  //-- free everything from Triangle, and manage myself the output
  if (in.numberofpoints != out.numberofpoints)
    std::clog << "INTERSECTIONS WHILE CDTing." << std::endl;
  for (int i = 0; i < out.numberofpoints; i++) 
    vertices.push_back(Point3(out.pointlist[i * 2], out.pointlist[i * 2 + 1], out.pointattributelist[i]));
  for (int i = 0; i < out.numberoftriangles; i++) {
    Triangle t;
    t.v0 = out.trianglelist[i * out.numberofcorners + 0];
    t.v1 = out.trianglelist[i * out.numberofcorners + 1];
    t.v2 = out.trianglelist[i * out.numberofcorners + 2];
    triangles.push_back(t);
  }

  free(in.pointlist);
  free(in.pointattributelist);
  free(in.segmentlist);

  if (in.numberofholes > 0)
    free(out.holelist);

  free(out.pointlist);
  free(out.pointattributelist);
  free(out.pointmarkerlist);
  free(out.trianglelist);
  free(out.triangleattributelist);
  free(out.neighborlist);
  free(out.segmentlist);
  free(out.segmentmarkerlist);
  // free(out.edgelist);
  // free(out.edgemarkerlist);
  return true;
}