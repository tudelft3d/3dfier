/*
  3dfier: takes 2D GIS datasets and "3dfies" to create 3D city models.
  
  Copyright (C) 2015-2016  3D geoinformation research group, TU Delft

  This file is part of 3dfier.

  3dfier is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  3dfier is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with 3difer.  If not, see <http://www.gnu.org/licenses/>.

  For any information or further details about the use of 3dfier, contact
  Hugo Ledoux 
  <h.ledoux@tudelft.nl>
  Faculty of Architecture & the Built Environment
  Delft University of Technology
  Julianalaan 134, Delft 2628BL, the Netherlands
*/


#ifndef geomtools_h
#define geomtools_h

#include "definitions.h"
#include <random>

void get_point_inside(Ring2& ring, Point2& p);
bool triangle_contains_segment(Triangle t, int a, int b);
bool getCDT(const Polygon2* pgn,
            const std::vector< std::vector<int> > &z, 
            std::set<Point3> &vertices, 
            std::vector<Triangle> &triangles, 
            const std::vector<Point3> &lidarpts = std::vector<Point3>());


#endif /* geomtools_h */
