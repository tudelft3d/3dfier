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

std::string gen_key_bucket(const Point2* p);
std::string gen_key_bucket(const Point3* p);
std::string gen_key_bucket(const Point3* p, float z);

double distance(const Point2 &p1, const Point2 &p2);
double sqr_distance(const Point2 &p1, const Point2 &p2);
double sqr_distance(const Point2 &p1, double x, double y);
bool   getCDT(Polygon2* pgn,
            const std::vector< std::vector<int> > &z, 
            std::vector< std::pair<Point3, std::string> > &vertices, 
            std::vector<Triangle> &triangles, 
            const std::vector<Point3> &lidarpts = std::vector<Point3>(),
            double tinsimp_threshold=0);

//--- Point-in-polygon grid
// Implementation of the grid center point algorithm by Li and Wang 2013
#define ND -1  // not determined
#define CBLACK 0
#define CWHITE 1
#define CSINGULAR 2

#define DONT_INTERSECT 0
#define DO_INTERSECT   1
#define BOUNDARY 2

class PolyEdge0 {
public:
  double x1, y1, x2, y2;
  //std::string wkt;
  PolyEdge0(double x1, double y1, double x2, double y2) {
    this->x1 = x1; this->y1 = y1; this->x2 = x2; this->y2 = y2;
    //this->wkt = "LINESTRING(" + std::to_string(x1) + " " + std::to_string(y1) + ", " + std::to_string(x2) + " " + std::to_string(y2) + ")";
  }
};

class Vertex {
public:
  double x;
  double y;
};

class PolyEdge {
public:
  int ring;
  int idx1;
  int idx2;
  Point2* v1;
  Point2* v2;
  //std::string wkt;
  PolyEdge(Point2* v1, Point2* v2, int ring, int idx1, int idx2) {
    this->v1 = v1, this->v2 = v2, this->ring = ring, this->idx1 = idx1, this->idx2 = idx2;
    //this->wkt = "LINESTRING(" + std::to_string(v1->x) + " " + std::to_string(v1->y) + ", " + std::to_string(v2->x) + " " + std::to_string(v2->y) + ")";
  }
};

class GridCell {
public:
  unsigned char color;
  std::vector<PolyEdge*> edges;
  std::vector<std::vector<Point2*>> vertices;
  GridCell() {
    color = ND;
  }
};

class Grid {
  std::vector<PolyEdge*> edges;
  Vertex*** vertices;
  double xmin, ymin, xmax, ymax;  // bounding box
  double sizex, sizey; // size cells in x and y
  GridCell*** cells;
  void constructEdges();
  void rasterize();
  void markCells();
  PolyEdge* getEdgeToCenter(int ix, int iy, double x, double y);
protected:
  Polygon2 * polygon;
public:
  int celllimit;
  int cellsx, cellsy;	// number of cells in x and y
  Grid(Polygon2* poly) {
    polygon = poly; celllimit = 30;
  }
  ~Grid();
  void prepare();
  bool checkPoint(double x, double y);
  std::set<PolyEdge*> getEdges(double x, double y, double radius);
  std::set<Point2*> getVertices(double x, double y, double radius);
  bool sqr_distance(double x, double y, double radius);
};

int RayLineIntersection(Point2* r1, Point2* r2, PolyEdge* e);
int HorizontalRayLineIntersection(Point2* r1, Point2* r2, PolyEdge* e);

#endif /* geomtools_h */
