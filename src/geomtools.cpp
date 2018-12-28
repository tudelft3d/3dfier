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

#include "geomtools.h"
#include "io.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Projection_traits_xy_3.h>
#include <CGAL/Triangulation_vertex_base_with_id_2.h>
#include <CGAL/Triangulation_face_base_with_info_2.h>
#include <CGAL/Polygon_2.h>

#include <vector>
#include <unordered_set>
#include <boost/heap/fibonacci_heap.hpp>

// fibonacci heap for greedy insertion code
struct point_error {
  point_error(int i, double e) : index(i), error(e){}
  int index;
  double error;
  
  bool operator<(point_error const & rhs) const
  {
    return error < rhs.error;
  }
};
typedef boost::heap::fibonacci_heap<point_error> Heap;
typedef Heap::handle_type heap_handle;

typedef CGAL::Exact_predicates_inexact_constructions_kernel			K;
typedef CGAL::Projection_traits_xy_3<K>								Gt;
typedef CGAL::Triangulation_vertex_base_with_id_2<Gt>				Vb;
struct FaceInfo2
{
  FaceInfo2() {}
  int nesting_level;
  bool in_domain() {
    return nesting_level % 2 == 1;
  }
  std::vector<heap_handle>* points_inside = nullptr;
  CGAL::Plane_3<K>* plane = nullptr;
};
typedef CGAL::Triangulation_face_base_with_info_2<FaceInfo2, Gt>	Fbb;
typedef CGAL::Constrained_triangulation_face_base_2<Gt, Fbb>		Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb>				Tds;
typedef CGAL::Exact_predicates_tag									Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<Gt, Tds, Itag>	CDT;
typedef CDT::Point													Point;
typedef CGAL::Polygon_2<Gt>											Polygon_2;

struct PointXYHash {
  std::size_t operator()(Point const& p) const noexcept {
    std::size_t h1 = std::hash<double>{}(p.x());
    std::size_t h2 = std::hash<double>{}(p.y());
    return h1 ^ (h2 << 1);
  }
};
struct PointXYEqual {
  bool operator()(Point const& p1, Point const& p2) const noexcept {
    auto ex = p1.x() == p2.x();
    auto ey = p1.y() == p2.y();
    return ex && ey;
  }
};

inline double compute_error(Point &p, CDT::Face_handle &face);
void greedy_insert(CDT &T, const std::vector<Point3> &pts, double threshold);

void mark_domains(CDT& ct,
  CDT::Face_handle start,
  int index,
  std::list<CDT::Edge>& border) {
  if (start->info().nesting_level != -1) {
    return;
  }
  std::list<CDT::Face_handle> queue;
  queue.push_back(start);
  while (!queue.empty()) {
    CDT::Face_handle fh = queue.front();
    queue.pop_front();
    if (fh->info().nesting_level == -1) {
      fh->info().nesting_level = index;
      for (int i = 0; i < 3; i++) {
        CDT::Edge e(fh, i);
        CDT::Face_handle n = fh->neighbor(i);
        if (n->info().nesting_level == -1) {
          if (ct.is_constrained(e)) border.push_back(e);
          else queue.push_back(n);
        }
      }
    }
  }
}

//explore set of facets connected with non constrained edges,
//and attribute to each such set a nesting level.
//We start from facets incident to the infinite vertex, with a nesting
//level of 0. Then we recursively consider the non-explored facets incident 
//to constrained edges bounding the former set and increase the nesting level by 1.
//Facets in the domain are those with an odd nesting level.
void mark_domains(CDT& cdt) {
  for (CDT::All_faces_iterator it = cdt.all_faces_begin(); it != cdt.all_faces_end(); ++it) {
    it->info().nesting_level = -1;
  }
  std::list<CDT::Edge> border;
  mark_domains(cdt, cdt.infinite_face(), 0, border);
  while (!border.empty()) {
    CDT::Edge e = border.front();
    border.pop_front();
    CDT::Face_handle n = e.first->neighbor(e.second);
    if (n->info().nesting_level == -1) {
      mark_domains(cdt, n, e.first->info().nesting_level + 1, border);
    }
  }
}

bool getCDT(Polygon2* pgn,
  const std::vector< std::vector<int> > &z,
  std::vector< std::pair<Point3, std::string> > &vertices,
  std::vector<Triangle> &triangles,
  const std::vector<Point3> &lidarpts,
  double tinsimp_threshold) {
  CDT cdt;

  //-- gather all rings
  std::vector<Ring2> rings;
  rings.push_back(pgn->outer());
  for (Ring2& iring : pgn->inners())
    rings.push_back(iring);

  Polygon_2 poly;
  int ringi = -1;
  for (Ring2& ring : rings) {
    ringi++;
    for (int i = 0; i < ring.size(); i++) {
      poly.push_back(Point(bg::get<0>(ring[i]), bg::get<1>(ring[i]), z_to_float(z[ringi][i])));
    }
    cdt.insert_constraint(poly.vertices_begin(), poly.vertices_end(), true);
    poly.clear();
  }

  //-- add the lidar points to the CDT, if any
  if (lidarpts.size() > 0) {
    if (tinsimp_threshold != 0)
      greedy_insert(cdt, lidarpts, tinsimp_threshold);
    else {
      for (auto &pt : lidarpts) {
        cdt.insert(Point(bg::get<0>(pt), bg::get<1>(pt), bg::get<2>(pt)));
      }
    }
  }

  //Mark facets that are inside the domain bounded by the polygon
  mark_domains(cdt);

  unsigned index = 0;
  int count = 0;

  if (!cdt.is_valid()) {
    throw std::runtime_error("CDT is invalid.");
  }
  for (CDT::Finite_vertices_iterator vit = cdt.finite_vertices_begin();
    vit != cdt.finite_vertices_end(); ++vit) {
    Point3 p = Point3(vit->point().x(), vit->point().y(), vit->point().z());
    vertices.push_back(std::make_pair(p, gen_key_bucket(&p)));
    vit->id() = index++;
  }

  for (CDT::Finite_faces_iterator fit = cdt.finite_faces_begin();
    fit != cdt.finite_faces_end(); ++fit) {
    if (fit->info().in_domain()) {
      Triangle t;
      t.v0 = fit->vertex(0)->id();
      t.v1 = fit->vertex(1)->id();
      t.v2 = fit->vertex(2)->id();
      triangles.push_back(t);
      count++;
    }
  }
  return true;
}

std::string gen_key_bucket(const Point2* p) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(3) << p->get<0>() << " " << p->get<1>();
  return ss.str();
}

std::string gen_key_bucket(const Point3* p) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(3) << p->get<0>() << " " << p->get<1>() << " " << std::setprecision(2) << p->get<2>();
  return ss.str();
}

std::string gen_key_bucket(const Point3* p, float z) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(3) << p->get<0>() << " " << p->get<1>() << " " << std::setprecision(2) << z;
  return ss.str();
}

double distance(const Point2 &p1, const Point2 &p2) {
  double dx = p1.x() - p2.x();
  double dy = p1.y() - p2.y();
  return sqrt(dx * dx + dy * dy);
}

double sqr_distance(const Point2 &p1, const Point2 &p2) {
  double dx = p1.x() - p2.x();
  double dy = p1.y() - p2.y();
  return dx * dx + dy * dy;
}

//--- TIN Simplification
// Greedy insertion/incremental refinement algorithm adapted from "Fast polygonal approximation of terrain and height fields" by Garland, Michael and Heckbert, Paul S.
inline double compute_error(Point &p, CDT::Face_handle &face) {
  if(!face->info().plane)
    face->info().plane = new CGAL::Plane_3<K>(
      face->vertex(0)->point(),
      face->vertex(1)->point(),
      face->vertex(2)->point());
  if(!face->info().points_inside)
    face->info().points_inside = new std::vector<heap_handle>();

  auto plane = face->info().plane;
  auto interpolate = - plane->a()/plane->c() * p.x() - plane->b()/plane->c()*p.y() - plane->d()/plane->c();
  double error = std::fabs(interpolate - p.z());
  return error;
}

void greedy_insert(CDT &T, const std::vector<Point3> &pts, double threshold) {
  // assumes all lidar points are inside a triangle
  Heap heap;

  // Convert all elevation points to CGAL points
  std::vector<Point> cpts;
  cpts.reserve(pts.size());
  for (auto& p : pts) {
    cpts.push_back(Point(bg::get<0>(p), bg::get<1>(p), bg::get<2>(p)));
  }

  // compute initial point errors, build heap, store point indices in triangles
  {
    std::unordered_set<Point, PointXYHash, PointXYEqual> set;
    for(int i=0; i<cpts.size(); i++){
      auto p = cpts[i];
      // detect and skip duplicate points
      auto not_duplicate = set.insert(p).second;
      if(not_duplicate){
        auto face = T.locate(p);
        auto e = compute_error(p, face);
        auto handle = heap.push(point_error(i,e));
        face->info().points_inside->push_back(handle);
      }
    }
  }
  
  // insert points, update errors of affected triangles until threshold error is reached
  while (!heap.empty() && heap.top().error > threshold){
    // get top element (with largest error) from heap
    auto maxelement = heap.top();
    auto max_p = cpts[maxelement.index];

    // get triangles that will change after inserting this max_p
    std::vector<CDT::Face_handle> faces;
    T.get_conflicts ( max_p, std::back_inserter(faces) );

    // insert max_p in triangulation
    auto face_hint = faces[0];
    auto v = T.insert(max_p, face_hint);
    face_hint = v->face();
    
    // update clear info of triangles that just changed, collect points that were inside these triangles
    std::vector<heap_handle> points_to_update;
    for (auto face : faces) {
      if (face->info().plane){
        delete face->info().plane;
        face->info().plane = nullptr;
      }
      if (face->info().points_inside) {
        for (auto h :*face->info().points_inside){
          if( maxelement.index != (*h).index)
            points_to_update.push_back(h);
        }
        face->info().points_inside->clear();
      }
    }
    
    // remove the point we just inserted in the triangulation from the heap
    heap.pop();

    // update the errors of affected elevation points
    for (auto curelement : points_to_update){
      auto p = cpts[(*curelement).index];
      auto containing_face = T.locate(p, face_hint);
      const double e = compute_error(p, containing_face);
      const point_error new_pe = point_error((*curelement).index, e);
      heap.update(curelement, new_pe);
      containing_face->info().points_inside->push_back(curelement);
    }
  }

  //cleanup the stuff I put in face info of triangles
  for (CDT::Finite_faces_iterator fit = T.finite_faces_begin();
    fit != T.finite_faces_end(); ++fit) {
      if (fit->info().plane){
        delete fit->info().plane;
        fit->info().plane = nullptr;
      }
      if (fit->info().points_inside) {
        delete fit->info().points_inside;
        fit->info().points_inside = nullptr;
      }
    }
}

//--- Point-in-polygon grid
// Implementation of the grid center point algorithm by Li and Wang 2013
int sgn(double val) {
  return (double(0) < val) - (val < double(0));
};

void Grid::prepare() {
  //-- Calculate Bbox
  Box2 bbox = bg::return_envelope<Box2>(*polygon);
  this->xmin = bg::get<bg::min_corner, 0>(bbox);
  this->xmax = bg::get<bg::max_corner, 0>(bbox);
  this->ymin = bg::get<bg::min_corner, 1>(bbox);
  this->ymax = bg::get<bg::max_corner, 1>(bbox);
  
  // add 1 percent to bbox to overcome finite arithmetics
  double a = 0.01 * (this->xmax - this->xmin);
  double b = 0.01 * (this->ymax - this->ymin);
  this->xmin -= a;
  this->xmax += a;
  this->ymin -= b;
  this->ymax += b;
  
  //-- Calculate cell size
  double deltax = (xmax - xmin);
  double deltay = (ymax - ymin);
  double ration = deltax / deltay;

  double sqrtnr = std::sqrt(totcells);
  cellsx = 2 * (1 + (int)(ration * sqrtnr));
  if (cellsx > celllimit) cellsx = celllimit;
  sizex = (xmax - xmin) / cellsx;

  cellsy = 2 * (1 + (int)(sqrtnr / ration));
  if (cellsy > celllimit) cellsy = celllimit;
  sizey = (ymax - ymin) / cellsy;

  //-- Create empty list of GridCells
  int i, j;
  cells = new GridCell**[cellsx];
  for (i = 0; i < cellsx; i++)
    cells[i] = new GridCell*[cellsy];

  for (i = 0; i < cellsx; i++)
    for (j = 0; j < cellsy; j++)
      cells[i][j] = new GridCell();

  rasterize();
  markCells();
}

void Grid::constructEdges() {
  for (int ringi = 0; ringi <= polygon->inners().size(); ringi++) {
    Ring2 r;
    if (ringi == 0) {
      r = polygon->outer();
    }
    else {
      r = polygon->inners()[ringi - 1];
    }
  
    int n = r.size();
    for (int i = 1; i <= n; i++) {
      this->edges.push_back(new PolyEdge(r[i - 1].x(), r[i - 1].y(), r[i].x(), r[i].y()));
    }
    this->edges.push_back(new PolyEdge(r[n].x(), r[n].y(), r[0].x(), r[0].y()));
  }
};

void Grid::rasterize(){
  for (PolyEdge* e : edges) {
    // Get direction vector
    double xlen = abs(e->x2 - e->x1);
    double ylen = abs(e->y2 - e->y1);

    // Normalize vector
    double normal = std::sqrt(xlen * xlen + ylen * ylen);
    double dirx = xlen / normal;
    double diry = ylen / normal;

    // Get direction sign
    int stepx = sgn(dirx);
    int stepy = sgn(diry);

    // Get starting cell grid location
    int xp = (int)((e->x1 - xmin) / sizex);
    int yp = (int)((e->y1 - ymin) / sizey);

    // Calculate length to next grid edge crossing
    double deltax, deltay;
    if (stepx == 0) {
      deltax = DBL_MAX;
    }
    else {
      deltax = sizex / std::abs(dirx);
    }

    if (stepy == 0) {
      deltay = DBL_MAX;
    }
    else {
      deltay = sizey / std::abs(diry);
    }

    double k = e->x1 - (xp * sizex + xmin);
    double dx = k * deltax / sizex;
    if (stepx != -1) {
      dx = deltax - dx;
    }

    k = e->y1 - (yp * sizey + ymin);
    double dy = k * deltay / sizey;
    if (stepy != -1) {
      dy = deltay - dy;
    }

    // Loop through cells untill the end of edge is reached
    int lastxp = -1;
    int lastyp = -1;
    while (lastxp != xp && lastyp != yp) {
      lastxp = xp;
      lastyp = yp;
      if (dx <= dy) {
        dx += deltax;
        xp += stepx;
        cells[xp][yp]->edges.push_back(e);
      }
      if (dy <= dx) {
        dy += deltay;
        yp += stepy;
        cells[xp][yp]->edges.push_back(e);
      }
    }
  }
}

int HorizontalRayLineIntersection(PolyEdge r, PolyEdge* e) {
  // line is completely above or below ray (only horizontal rays are allowed)
  if ((e->y1 < r.y1 && e->y2 < r.y1) || (e->y1 > r.y1 && e->y2 > r.y1)) {
    return 0;
  }
  double a1 = r.y2 - r.y1;
  double b1 = r.x2 - r.x1;
  double c1 = a1 * (r.x1) + b1 * (r.y1);

  double a2 = e->y2 - e->y1;
  double b2 = e->x2 - e->x1;
  double c2 = a1 * (e->x1) + b1 * (e->y1);

  double determinant = a1 * b2 - a2 * b1;

  if (determinant == 0) {
    // The lines are parallel. 
    return 0;
  }
  else {
    double x = (b2*c1 - b1*c2) / determinant;
    double y = (a1*c2 - a2*c1) / determinant;
    // Check if the ray end (center of cell) is on intersection, in that case cell is GRAY
    if (x == r.x2 && y == r.y2) {
      return -1;
    }
    return 1;
  }
}

void Grid::markCells() {
  //-- Calculate value of GridCells center point
  int prevcolor = CBLACK;
  double x = xmin - (sizex / 2);
  double y = ymin - (sizey / 2);
  for (int i = 0; i < cellsx; i++) {
    for (int j = 0; j < cellsy; j++) {
      // store previous x,y
      double prevx = x;
      double prevy = y;

      // Reset prev color to black for each new row
      if (i = 0) {
        prevcolor = CBLACK;
        x = xmin + (sizex * i) - (sizex / 2);
      }

      // increase y-coordinate of cell
      y += sizey;

      //raytracing from center to center. Use edges from this and previous cell if not boundary
      int crossings = 0;
      int color = CBLACK;
      PolyEdge r = PolyEdge(prevx, prevy, x, y);
      // Iterate edges of current cell
      for (PolyEdge* e : cells[i][j]->edges) {
        int cross = HorizontalRayLineIntersection(r, e);
        if (cross == -1) {
          color = CSINGULAR;
          break;
        }
        crossings += cross;
      }
      // Iterate edges of previous cell
      if (color != CSINGULAR && j > 0) {
        for (PolyEdge* e : cells[i][j - 1]->edges) {
          int cross = HorizontalRayLineIntersection(r, e);
          if (cross == -1) {
            color = CSINGULAR;
            break;
          }
          crossings += cross;
        }
      }
      if (color != CSINGULAR) {
        // equal crossings gives same color for cell
        if (crossings % 0 == 0) {
          color = prevcolor;
        }
        else {
          // inverse color
          if (prevcolor == CBLACK) {
            color = CWHITE;
          }
          else {
            color = CBLACK;
          }
        }
        prevcolor = color;
      }
      // Store color
      cells[i][j]->color = color;
    }
    // increase x-coordinate of cell
    x += sizex;
  }
}


////--- Point-in-polygon grid
//// Implementation of the grid center point algorithm by Zalik and Kolingerova
//
//void prepareGrid(Polygon2* poly) {
//  Grid g = Grid(poly);
//  g.prepare();
//}
//
//void Grid::prepare() {
//  calculateBbox();
//  calculateSize();
//  constructListOfEdges();
//
//  rasterize();
//  markCells();
//}
//
//void Grid::calculateBbox() {
//  double	vx0, vy0;
//  // Determine bbox
//  for (int i = 1; i < this->polygon->outer().size(); i++) {
//    vx0 = this->polygon->outer()[i].x();
//    if (this->xmin > vx0) {
//      this->xmin = vx0;
//    }
//    else if (this->xmax < vx0) {
//      this->xmax = vx0;
//    }
//
//    vy0 = this->polygon->outer()[i].y();
//    if (this->ymin > vy0) {
//      this->ymin = vy0;
//    }
//    else if (this->ymax < vy0) {
//      this->ymax = vy0;
//    }
//  }
//
//  // add 1 percent to bbox to overcome finite arithmetics
//  double a = 0.01 * (this->xmax - this->xmin);
//  double b = 0.01 * (this->ymax - this->ymin);
//  this->xmin -= a;
//  this->xmax += a;
//  this->ymin -= b;
//  this->ymax += b;
//}
//
//void Grid::calculateSize() {
//  int CellLimit = 20;
//  double deltax = (this->xmax - this->xmin);
//  double deltay = (this->ymax - this->ymin);
//  double ratio = deltax / deltay;
//
//  double SqrNr = sqrt(bg::num_points(this->polygon));
//  this->cellsx = 2 * (1 + (int)(ratio * SqrNr));
//  if (this->cellsx > CellLimit) {
//    this->cellsx = CellLimit;
//  }
//  this->sizex = (xmax - xmin) / this->cellsx;
//
//  this->sizey = 2 * (1 + (int)(SqrNr / ratio));
//  if (this->sizey > CellLimit) {
//    this->sizey = CellLimit;
//  }
//  this->cellsy = (ymax - ymin) / this->sizey;
//
//  int i, j;
//  this->cells = new GridCell**[this->sizex];
//  for (i = 0; i < this->sizex; i++)
//    this->cells[i] = new GridCell*[this->sizey];
//
//  for (i = 0; i < this->sizex; i++)
//    for (j = 0; j < this->sizey; j++)
//      this->cells[i][j] = new GridCell();
//}
//
//void Grid::constructListOfEdges() {
//  Polygon2* poly = polygon;
//  Point* p;
//  int i, n;
//  EdgeList* elst;
//
//  for (int ringi = 0; ringi <= poly->inners().size(); ringi++) {
//    Ring2 r;
//    if (ringi == 0) {
//      r = poly->outer();
//    }
//    else {
//      r = poly->inners()[ringi - 1];
//    }
//
//    n = r.size();
//    for (i = 1; i <= n; i++) {
//      elst = new EdgeList(gpoint{ r[i - 1].x(),r[i - 1].y() }, gpoint{ r[i].x(), r[i].y() });
//      elst->Next = edges;
//      edges = elst;
//    }
//    elst = new EdgeList(gpoint{ r[n].x(),r[n].y() }, gpoint{ r[0].x(), r[0].y() });
//    elst->Next = edges;
//    edges = elst;
//  }
//}
//
//void Grid::rasterize() {
//  double diagonal = (sqrt((xmax - xmin)*(xmax - xmin) + (ymax - ymin)*(ymax - ymin)));
//  CWLineTraversal* r = new CWLineTraversal(xmin, ymin, cellsx, cellsy, diagonal);
//
//  int xp, yp;
//  gpoint p1, p2;
//
//  EdgeList* elst = edges;
//  while (elst != NULL) {
//    elst->ReturnEdge(p1, p2);
//    r->setEdge(&p1, &p2);
//    while (r->returnNextCellPosition(xp, yp))
//      if ((xp >= 0) && (xp < cellsx) &&
//        (yp >= 0) && (yp < cellsy)) {
//        cells[xp][yp]->addEdge(elst);
//      }
//    elst = elst->Next;
//  }
//
//  delete r;
//}
//
//void Grid::markCells()   {
// int i, j;
//
//    flood = new MyFloodFill(NoOfCellsX, NoOfCellsY, Lattice, WHITE);
//
//    // beginning of filling outher cells
//    for (i = 0; i < NoOfCellsY; i++)       {
// if (Lattice[0][i]->ReturnBWG() == ND)
//        flood->DoFlood(0, i);
//      }
//
//      for (i = NoOfCellsY - 1; i >= 0; i--)         {
// if (Lattice[NoOfCellsX-1][i]->ReturnBWG() == ND)
//          flood->DoFlood(NoOfCellsX - 1, i);
//        }
//
//        for (i = 0; i < NoOfCellsX; i++)           {
// if (Lattice[i][0]->ReturnBWG() == ND)
//            flood->DoFlood(i, 0);
//          }
//
//          for (i = NoOfCellsX - 1; i >= 0; i--)             {
// if (Lattice[i][NoOfCellsY-1]->ReturnBWG() == ND)
//              flood->DoFlood(i, NoOfCellsY - 1);
//            }
//            // end filling outher cells
//
//            for (i = 1; i < NoOfCellsX - 1; i++)               {
// for (j = 1; j < NoOfCellsY-1; j++)
//                if (Lattice[i][j]->ReturnBWG() == ND) { // first cell to the left is for sure gray. Let us determine if
//                                                        // we are inside the loop or the hole. For this purposes the shortest distance 
//                                                        // to the line segment to the left cell is detrmine. If it belongs to loop
//                                                        // we are inside loop otherwise inside ring;
//                  if (InsideLoop(i, j) == 1)
//                    flood->SetFillColor(BLACK);
//                  else
//                    flood->SetFillColor(WHITE);
//                  flood->DoFlood(i, j);
//                }
//              }
//
//            }
//
//template <typename T> int sgn(T val) {
//  return (T(0) < val) - (val < T(0));
//}
//
//void CWLineTraversal::setEdge(gpoint* q1, gpoint* q2)
//// Initialization of the Cleary and Wyvill algorithm
//{
//  double a = q2->x - q1->x;
//  double b = q2->y - q1->y;
//
//  PixelNo = 0;
//  direction.x = a;  direction.y = b;
//  startp.x = q1->x; startp.y = q1->y;
//  endp.x = q2->x;   endp.y = q2->y;
//
//  // calculate the ray lenght between x, y and calculate initial distances of the ray
//
//  double norm = sqrt((a*a) + (b*b));
//  direction.x = direction.x / norm;
//  direction.y = direction.y / norm;
//
//  stepx = sgn(direction.x);
//  stepy = sgn(direction.y);
//
//  startxp = (int)((startp.x - xmin) / stepx);
//  startyp = (int)((startp.y - ymin) / stepy);
//
//  endxp = (int)((endp.x - xmax) / stepx);
//  endyp = (int)((endp.y - ymax) / stepy);
//
//  firstcell = false;
//
//  if (stepx == 0) deltax = std::numeric_limits<double>::max();
//  else deltax = stepx / abs(direction.x);
//
//  if (stepy == 0) deltay = std::numeric_limits<double>::max();
//  else deltay = stepy / abs(direction.y);
//
//  double k = q1->x - (startxp * stepx + xmin);
//  dx = k * deltax / stepx;
//  if (stepx != -1) dx = deltax - dx;
//
//  k = q1->y - (startyp * stepy + ymin;
//  dy = k * deltay / stepy;
//  if (stepy != -1) dy = deltay - dy;
//}
//
//bool CWLineTraversal::returnNextCellPosition(int& xp, int& yp) {
//  if (PixelNo++ > diagonal) return false;  // just in case the traversel goes wrong
//
//  if (firstcell == false) {
//    xp = startxp; yp = startyp;
//    firstcell = true;
//    return true;
//  }
//
//  if ((xp == endxp) && (yp == endyp)) {
//    firstcell = false;
//    return false;
//  }
//
//  // core of Cleary-Wyvill algorithm
//  if (dx <= dy) {
//    dx += deltax;
//    xp += stepx;
//    return true;
//  }
//  if (dy <= dx) {
//    dy += deltay;
//    yp += stepy;
//    return true;
//  }
//  return false;
//}
//
//void GridCell::addEdge(EdgeList* e) {
//  e->Next = edges;
//  edges = e;
//  BWGS = GRAY;
//}