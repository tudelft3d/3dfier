//
//  geomtools.hpp
//  boo
//
//  Created by Hugo Ledoux on 23/09/15.
//  Copyright © 2015 Hugo Ledoux. All rights reserved.
//

#ifndef geomtools_h
#define geomtools_h

#include "definitions.h"
#include <random>

//-- stuff for Shewchuk's Triangle
#define ANSI_DECLARATORS
#define VOID void
#define REAL double

extern "C" {
  #include "triangle.h"
}

bool getCDT(Polygon2d* p, std::vector<Point3d> &vertices, std::vector<Triangle> &triangles);
void get_point_inside(Ring2d& ring, Point2d& p);

#endif /* geomtools_h */
