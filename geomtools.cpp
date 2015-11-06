//
//  geomtools.cpp
//
//  Created by Hugo Ledoux on 23/09/15.
//  Copyright © 2015 Hugo Ledoux. All rights reserved.
//

#include "geomtools.h"


int polygon2_find_point(Polygon2* pgn, Point2& p) {
  double threshold = 0.001;
  Ring2 oring = bg::exterior_ring(*pgn);
  int re = -1;
  for (int i = 0; i < oring.size(); i++) {
    if (bg::distance(p, oring[i]) <= threshold)
      return i;
  }
  int offset = int(bg::num_points(oring));
  auto irings = bg::interior_rings(*pgn);
  for (Ring2& iring: irings) {
    for (int i = 0; i < iring.size(); i++) {
      if (bg::distance(p, iring) <= threshold)
        return (i + offset);
    }
    offset += bg::num_points(iring);
  }
  return re;
}


bool polygon2_find_segment(Polygon2* pgn, Point2& a, Point2& b) {
  int posa = polygon2_find_point(pgn, a);
  if (posa != -1) {
    Point2 tmp;
    tmp = polygon2_get_point_before(pgn, posa);
    if (bg::equals(b, tmp) == true)
      return true;
  }
  return false;
}


Point2& polygon2_get_point(Polygon2* pgn, int i) {
  Ring2 oring = bg::exterior_ring(*pgn);
  int offset = int(bg::num_points(oring));
  if (i < offset) 
    return oring[i];
  auto irings = bg::interior_rings(*pgn);
  for (Ring2& iring: irings) {
    if (i < (offset + iring.size()))
      return (iring[i - offset]);
    offset += iring.size();
  }
  Point2 tmp;
  return tmp;
}

Point2& polygon2_get_point_before(Polygon2* pgn, int i) {
  Ring2 oring = bg::exterior_ring(*pgn);
  int offset = int(bg::num_points(oring));
  if (i < offset) {
    if (i == 0)
      return oring.back();
    else
      return oring[i - 1];
  }
  auto irings = bg::interior_rings(*pgn);
  for (Ring2& iring: irings) {
    if (i < (offset + iring.size())) {
      if (i == offset)
        return iring.back();
      else
        return (iring[i - offset]);
    }
    offset += iring.size();
  }
  Point2 tmp;
  return tmp;
}




//-- TODO: cheap code that randomly generates points if centroid is not inside. to update asap.
void get_point_inside(Ring3& ring3, Point2& p) {
  Ring2 ring;
  for (auto& v : ring3)
    bg::append(ring, Point2(bg::get<0>(v), bg::get<1>(v)));
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




// 
bool getCDT(const Polygon3* pgn, 
            std::vector<Point3> &vertices, 
            std::vector<Triangle> &triangles, 
            std::vector<Segment> &segments, 
            const std::vector<Point3> &lidarpts) {
  Ring3 oring = bg::exterior_ring(*pgn);
  auto irings = bg::interior_rings(*pgn);
  struct triangulateio in, out;
  in.numberofpointattributes = 1;
  in.numberofpoints = int(bg::num_points(*pgn));
  in.numberofpoints += lidarpts.size();
  in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
  in.pointattributelist = (REAL *) malloc(in.numberofpoints * in.numberofpointattributes * sizeof(REAL));
  int counter = 0;
  //-- oring
  for (int i = 0; i < oring.size(); i++) {
    in.pointlist[counter++]  = bg::get<0>(oring[i]);
    in.pointlist[counter++]  = bg::get<1>(oring[i]);
    in.pointattributelist[i] = bg::get<2>(oring[i]);
  }
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
       in.pointattributelist[(counter / 2) - 1] = bg::get<2>(iring[i]);
      }
      Point2 pinside;
      get_point_inside(iring, pinside);
      // std::cout << "pinside: " << bg::wkt(pinside) << std::endl;
      in.holelist[holecount++] = bg::get<0>(pinside);
      in.holelist[holecount++] = bg::get<1>(pinside);
      in.pointattributelist[(counter / 2) - 1] = 0.0;
    }
  }
  //-- add the lidar points to the CDT, if any
  for (auto &pt : lidarpts) {
    in.pointlist[counter++] = bg::get<0>(pt);
    in.pointlist[counter++] = bg::get<1>(pt); 
    in.pointattributelist[(counter / 2) - 1] = bg::get<2>(pt);
  }
  in.pointmarkerlist = NULL;
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
    start += (iring.size() - 1);
  }
  in.segmentmarkerlist = NULL;
  //-- output from Triangle
  out.pointlist = (REAL *) NULL;
  out.pointattributelist = (REAL *) NULL;
  out.pointmarkerlist = (int *) NULL;
  out.trianglelist = (int *) NULL;
  out.triangleattributelist = (REAL *) NULL;
  out.neighborlist = (int *) NULL;
  out.segmentlist = (int *) NULL;
  out.segmentmarkerlist = (int *) NULL;
  out.edgelist = (int *) NULL;
  out.edgemarkerlist = (int *) NULL;
  //-- call Triangle
  triangulate((char *)"pzQ", &in, &out, NULL);
  //-- free everything from Triangle, and manage myself the output
  if (in.numberofpoints != out.numberofpoints)
    std::cout << "INTERSECTIONS WHILE CDTing." << std::endl;
  for (int i = 0; i < out.numberofpoints; i++) 
    vertices.push_back(Point3(out.pointlist[i * 2], out.pointlist[i * 2 + 1], out.pointattributelist[i]));
  for (int i = 0; i < out.numberoftriangles; i++) {
    Triangle t;
    t.v0 = out.trianglelist[i * out.numberofcorners + 0];
    t.v1 = out.trianglelist[i * out.numberofcorners + 1];
    t.v2 = out.trianglelist[i * out.numberofcorners + 2];
    triangles.push_back(t);
  }
  for (int i = 0; i < out.numberofsegments; i++) {
    Segment s;
    s.v0 = out.segmentlist[i * 2];
    s.v1 = out.segmentlist[i * 2 + 1];
    segments.push_back(s);
  }

  free(in.pointlist);
  free(in.pointmarkerlist);
  free(in.segmentlist);
  if (in.numberofholes > 0)
    free(in.holelist);
  free(out.pointlist);
  free(out.pointattributelist);
  free(out.pointmarkerlist);
  free(out.trianglelist);
  free(out.triangleattributelist);
  free(out.segmentlist);
  free(out.segmentmarkerlist);
  free(out.edgelist);
  free(out.edgemarkerlist);
  return true;
}