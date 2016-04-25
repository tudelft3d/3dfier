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


void get_point_inside(Ring2& ring, Point2& p);
bool triangle_contains_segment(Triangle t, int a, int b);
bool getCDT(const Polygon2* pgn,
            const std::vector< std::vector<int> > &z, 
            std::vector<Point3> &vertices, 
            std::vector<Triangle> &triangles, 
            const std::vector<Point3> &lidarpts = std::vector<Point3>());


#endif /* geomtools_h */
