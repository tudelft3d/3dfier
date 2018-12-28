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

class PolyEdge {
public:
  double x1, y1, x2, y2;
  PolyEdge(double x1, double y1, double x2, double y2) {
    x1 = x1; y1 = y1; x2 = x2; y2 = y2;
  }
};

class GridCell {
public:
  int color;
  std::vector<PolyEdge*> edges;
  GridCell() {
    color = ND;
  }
};

class Grid {
  Polygon2* polygon;
  std::vector<PolyEdge*> edges;
  int cellsx, cellsy;	// number of cells in x and y
  int totcells;	// cellsx * cellsy
  double xmin, ymin, xmax, ymax;  // bounding box
  double sizex, sizey; // size cells in x and y
  GridCell ***cells;
  int celllimit;
public:
  Grid(Polygon2* poly) {
    polygon = poly; celllimit = 100;
  }
  void prepare();
  void constructEdges();
  void rasterize();
  void markCells();
  bool checkPoint(double x, double y);
  PolyEdge getEdgeToCenter(int i, int j, double x, double y);
};


//#define ND -1  // not determined
//#define BLACK 0
//#define WHITE 1
//#define GRAY 2
//#define SINGULAR 3
//
//class EdgeList {
//  gpoint p1, p2;
//public:
//  EdgeList* Next;
//  EdgeList(gpoint& a, gpoint& b) {p1 = a; p2 = b; Next = NULL;}
//  void ReturnEdge(gpoint& a, gpoint& b) {a = p1; b = p2;}
//};
//
//typedef struct {
//  double x, y;
//} gpoint;
//
//class Grid {
//  Polygon2* polygon;
//  EdgeList* edges;
//  int cellsx, cellsy;	// gridcell size
//  int totcells;	// cellsx * cellsy
//  double xmin, ymin, xmax, ymax;  // bounding box
//  double sizex, sizey; // number of cells in x and y
//  GridCell ***cells;
//
//public:
//  Grid(Polygon2* poly) {this->polygon = poly;}
//  void prepare();
//  void calculateBbox();
//  void calculateSize();
//  void constructListOfEdges();
//  void rasterize();
//  void markCells();
//};
//
//class GridCell {
//  int BWGS; // black white gray singular
//  EdgeList* edges;
//public:
//  GridCell() {edges = NULL; BWGS = ND;};
//  void addEdge(EdgeList* e);
//};
//
//class CWLineTraversal {
//protected:
//  double stepy, stepx; // dimension of one cell
//  double xmin, ymin;   // origin of the grid
//  int PixelNo;         // number of "pixels" obtain by rasterization 
//private:
//  gpoint direction, startp, endp;
//  int signx, signy, signz;
//  double deltax, deltay, deltaz, dx, dy, dz, norma;
//  int firstcell;
//  double diagonal;
//  unsigned short startxp, startyp, endxp, endyp;
//public:
//  CWLineTraversal(double xmin, double ymin, double sx, double sy, double d) {xmin = xmin, ymin = ymin, stepx = sx, stepy = sy, diagonal = d};
//  void setEdge(gpoint*, gpoint*);
//  bool returnNextCellPosition(int&, int&);
//};

#endif /* geomtools_h */
