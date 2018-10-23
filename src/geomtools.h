/*
  3dfier: takes 2D GIS datasets and "3dfies" to create 3D city models.
  
  Copyright (C) 2015-2018  3D geoinformation research group, TU Delft

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

#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::Simple_cartesian<double>                    Kc;
typedef Kc::Point_3                                       Point3D;
typedef Kc::Triangle_3                                    Triangle3D;
typedef std::list<Triangle3D>::iterator                   AABB_Iterator;
typedef CGAL::AABB_triangle_primitive<Kc,AABB_Iterator>   AABB_Primitive;
typedef CGAL::AABB_traits<Kc, AABB_Primitive>             AABB_triangle_traits;
typedef CGAL::AABB_tree<AABB_triangle_traits>             AABB_Tree;

std::string gen_key_bucket(const Point2* p);
std::string gen_key_bucket(const Point3* p);
std::string gen_key_bucket(const Point3* p, float z);

double distance(const Point2 &p1, const Point2 &p2);
double sqr_distance(const Point2 &p1, const Point2 &p2);
double distance_3d(AABB_Tree const& TriTree, liblas::Point const& laspt);
bool   getCDT(Polygon2* pgn,
            const std::vector< std::vector<int> > &z, 
            std::vector< std::pair<Point3, std::string> > &vertices, 
            std::vector<Triangle> &triangles, 
            const std::vector<Point3> &lidarpts = std::vector<Point3>(),
            double tinsimp_threshold=0);

#endif /* geomtools_h */
