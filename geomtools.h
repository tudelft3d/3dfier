//
//  geomtools.hpp
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

void get_point_inside(Ring3& ring, Point2& p);
bool getCDT(const Polygon3* p, 
            std::vector<Point3> &vertices, 
            std::vector<Triangle> &triangles, 
            std::vector<Segment> &segments, 
            const std::vector<Point3> &lidarpts = std::vector<Point3>());


#endif /* geomtools_h */
