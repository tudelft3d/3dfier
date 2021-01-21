/*
  3dfier: takes 2D GIS datasets and "3dfies" to create 3D city models.
  
  Copyright (C) 2015-2020 3D geoinformation research group, TU Delft

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

std::string gen_key_bucket(const Point2* p);
std::string gen_key_bucket(const Point3* p);
std::string gen_key_bucket(const Point3* p, float z);

double distance(const Point2 &p1, const Point2 &p2);
double sqr_distance(const Point2 &p1, const Point2 &p2);
bool   getCDT(Polygon2* pgn,
            const std::vector< std::vector<int> > &z, 
            std::vector< std::pair<Point3, std::string> > &vertices, 
            std::vector<Triangle> &triangles, 
            const std::vector<Point3> &lidarpts = std::vector<Point3>(),
            double tinsimp_threshold=0);

struct PointSTL {
    PointSTL();
    PointSTL(const std::string& point);
    ~PointSTL();

    std::vector<double> vertex;

    std::array<double, 3> operator+(const PointSTL& other) const;
    std::array<double, 3> operator-(const PointSTL& other) const;
};

struct TriangleSTL {
    TriangleSTL();
    TriangleSTL(const PointSTL& pointa, const PointSTL& pointb, const PointSTL& pointc);
    ~TriangleSTL();

    PointSTL a, b, c;
    std::array<double, 3> vecU, vecV;

    std::array<double, 3> norm() const;
};

#endif /* geomtools_h */
