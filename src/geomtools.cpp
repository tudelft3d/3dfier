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

double sqr_distance(const Point2 &p1, double x, double y) {
  double dx = p1.x() - x;
  double dy = p1.y() - y;
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
  //-- Add edges to the grid
  constructEdges();
  
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

  double sqrtnr = std::sqrt(edges.size());
  cellsx = 2 * (1 + std::floor(ration * sqrtnr));
  if (cellsx > celllimit) cellsx = celllimit;
  sizex = (xmax - xmin) / cellsx;

  cellsy = 2 * (1 + std::floor(sqrtnr / ration));
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
  
    int n = r.size() - 1;
    for (int i = 1; i <= n; i++) {
      Point2* p = &r[i - 1];
      this->edges.push_back(new PolyEdge(&r[i-1],  &r[i], ringi, i-1, i));
      //this->edges.push_back(new PolyEdge(r[i - 1].x(), r[i - 1].y(), r[i].x(), r[i].y()));
    }
    this->edges.push_back(new PolyEdge(&r[n], &r[0], ringi, n, 0));
    //this->edges.push_back(new PolyEdge(r[n].x(), r[n].y(), r[0].x(), r[0].y()));
  }
};

void Grid::rasterize(){
  for (PolyEdge* e : edges) {
    // Get direction vector
    double xlen = e->v2->x() - e->v1->x();
    double ylen = e->v2->y() - e->v1->y();

    // Normalize vector
    double normal = std::sqrt(xlen * xlen + ylen * ylen);
    double dirx = xlen / normal;
    double diry = ylen / normal;

    // Get direction sign
    int stepx = sgn(dirx);
    int stepy = sgn(diry);

    // Get start cell grid location
    int xp = (int)((e->v1->x() - xmin) / sizex);
    int yp = (int)((e->v1->y() - ymin) / sizey);
    // Add edge to start cell
    cells[xp][yp]->edges.push_back(e);

    // Get end cell grid location
    int lastxp = (int)((e->v2->x() - xmin) / sizex);
    int lastyp = (int)((e->v2->y() - ymin) / sizey);

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

    double k = e->v1->x() - (xp * sizex + xmin);
    double dx = k * deltax / sizex;
    if (stepx != -1) {
      dx = deltax - dx;
    }
    if (stepx == 0) {
      dx = DBL_MAX;
    }

    k = e->v1->y() - (yp * sizey + ymin);
    double dy = k * deltay / sizey;
    if (stepy != -1) {
      dy = deltay - dy;
    }
    if (stepy == 0) {
      dy = DBL_MAX;
    }

    // Loop through cells untill the end of edge is reached
    while (lastxp != xp || lastyp != yp) {
      if (dx <= dy) {
        dx += deltax;
        xp += stepx;
      }
      else if (dy <= dx) {
        dy += deltay;
        yp += stepy;
      }
      cells[xp][yp]->edges.push_back(e);
    }
  }
}

/* Faster Line Segment Intersection
   Franklin Antonio
   Code from Graphic Gems III */
int RayLineIntersection(Point2* r1, Point2* r2, PolyEdge* e) {
  double Ax, Bx, Cx, Ay, By, Cy, d, ee, f, num, offset, x1lo, x1hi, y1lo, y1hi;

  Ax = r2->x() - r1->x();
  Bx = e->v1->x() - e->v2->x();

  if (Ax < 0) {						/* X bound box test*/
    x1lo = r2->x(); x1hi = r1->x();
  }
  else {
    x1hi = r2->x(); x1lo = r1->x();
  }
  if (Bx > 0) {
    if (x1hi < e->v2->x() || e->v1->x() < x1lo) return DONT_INTERSECT;
  }
  else {
    if (x1hi < e->v1->x() || e->v2->x() < x1lo) return DONT_INTERSECT;
  }

  Ay = r2->y() - r1->y();
  By = e->v1->y() - e->v2->y();

  if (Ay < 0) {						/* Y bound box test*/
    y1lo = r2->y(); y1hi = r1->y();
  }
  else {
    y1hi = r2->y(); y1lo = r1->y();
  }
  if (By > 0) {
    if (y1hi < e->v2->y() || e->v1->y() < y1lo) return DONT_INTERSECT;
  }
  else {
    if (y1hi < e->v1->y() || e->v2->y() < y1lo) return DONT_INTERSECT;
  }

  f = Ay * Bx - Ax * By;					/* both denominator*/
  if (f == 0) return DONT_INTERSECT;

  Cx = r1->x() - e->v1->x();
  Cy = r1->y() - e->v1->y();

  d = By * Cx - Bx * Cy;					/* alpha numerator*/
  if (f > 0) {						/* alpha tests*/
    if (d<0 || d>f) return DONT_INTERSECT;
  }
  else {
    if (d > 0 || d < f) return DONT_INTERSECT;
  }

  ee = Ax * Cy - Ay * Cx;					/* beta numerator*/
  if (f > 0) {						/* beta tests*/
    if (ee<0 || ee>f) return DONT_INTERSECT;
  }
  else {
    if (ee > 0 || ee < f) return DONT_INTERSECT;
  }

  ///*compute intersection coordinates*/
  //num = d * Ax;						/* numerator */
  //double x = r1->x() + num / f;		    /* intersection x */

  //num = d * Ay;
  //double y = r1->y() + num / f;				/* intersection y */

  //// Check if the ray end (center of cell) is on intersection, in that case cell is GRAY
  //if (x == r2->x() && y == r2->y()) {
  //  return BOUNDARY;
  //}

  return DO_INTERSECT;
}

int HorizontalRayLineIntersection(Point2* r1, Point2* r2, PolyEdge* e) {
  double Ax, Bx, Cx, By, Cy, d, ee, f, num, offset, x1lo, x1hi;

  /* Y bound box test*/
  By = e->v1->y() - e->v2->y();
  if (r1->y() < e->v1->y() && r1->y() < e->v2->y()) return DONT_INTERSECT;
  if (r1->y() > e->v1->y() && r1->y() > e->v2->y()) return DONT_INTERSECT;
  
  /* X bound box test*/
  Bx = e->v1->x() - e->v2->x();
  x1lo = r1->x(); x1hi = r2->x(); /* X bound box test*/
  if (Bx > 0) {
    if (x1hi < e->v2->x() || e->v1->x() < x1lo) return DONT_INTERSECT;
  }
  else {
    if (x1hi < e->v1->x() || e->v2->x() < x1lo) return DONT_INTERSECT;
  }

  // Ax is allways positive, ray goes from left to right
  Ax = r2->x() - r1->x();

  f = -Ax * By;					  /* both denominator*/
  Cx = r1->x() - e->v1->x();
  Cy = r1->y() - e->v1->y();

  d = By * Cx - Bx * Cy;	/* alpha numerator*/
  if (f > 0) {						/* alpha tests*/
    if (d<0 || d>f) return DONT_INTERSECT;
  }
  else {
    if (d > 0 || d < f) return DONT_INTERSECT;
  }

  ee = Ax * Cy;					  /* beta numerator*/
  if (f > 0) {						/* beta tests*/
    if (ee<0 || ee>f) return DONT_INTERSECT;
  }
  else {
    if (ee > 0 || ee < f) return DONT_INTERSECT;
  }

  /* intersection x */
  double x = r1->x() + d * Ax / f;

  // Check if the ray end (center of cell) is on intersection using epsilon
  // only test for x since ray is horizontal
  if (std::abs(x - r2->x()) < 0.000000001) {
    return BOUNDARY;
  }

  return DO_INTERSECT;
}

void Grid::markCells() {
  //-- Calculate value of GridCells center point
  int prevcolor = CBLACK;
  // start at x = -1/2 cell and y = 1/2 cell
  double x = xmin - (sizex / 2);
  double y = ymin + (sizey / 2);
  for (int iy = 0; iy < cellsy; iy++) {
    for (int ix = 0; ix < cellsx; ix++) {
      // Reset prev color to black for each new row
      if (ix == 0) {
        prevcolor = CBLACK;
        x = xmin + (sizex * (ix-0.5));
      }

      // store previous x,y
      double prevx = x;
      double prevy = y;

      // increase x-coordinate of cell
      x += sizex;

      // raytracing from center to center. Use edges from this and previous cell if not boundary
      int crossings = 0;
      int color = CBLACK;
      Point2* r1 = new Point2(prevx, prevy);
      Point2* r2 = new Point2(x, y);
      
      // create a set with unique edges add edges of current cell and previous cell
      // skip previous if cell is first in row
      std::set<PolyEdge*> edges(cells[ix][iy]->edges.begin(), cells[ix][iy]->edges.end());
      if (ix > 0) {
        edges.insert(cells[ix - 1][iy]->edges.begin(), cells[ix - 1][iy]->edges.end());
      }

      // iterate edges and do ray-line intersection
      for (PolyEdge* e : edges) {
        int cross = HorizontalRayLineIntersection(r1, r2, e);
        ///// TESTING
        //if (cross != RayLineIntersection(r, e)) {
        //  std::cout << "RayLineIntersection different" << std::endl;
        //}
        ///// TESTING
        if (cross == BOUNDARY) {
          color = CSINGULAR;
          break;
        }
        crossings += cross;
      }
      delete r1, r2;

      // calculate color of cell using crossing
      // and handle singular case (inverse previous color for next iteration)
      if (color == CSINGULAR) {
        // in singular inverse previous color
        if (prevcolor == CBLACK) {
          prevcolor = CWHITE;
        }
        else {
          prevcolor = CBLACK;
        }
      }
      else
      {
        // even crossings gives same color for cell
        if (crossings % 2 == 0) {
          color = prevcolor;
        }
        else {
          // inverse color if uneven crossings
          if (prevcolor == CBLACK) {
            color = CWHITE;
          }
          else {
            color = CBLACK;
          }
        }
        // store color for next pass
        prevcolor = color;
      }
      // Store color in cell
      cells[ix][iy]->color = color;
    }
    // increase y-coordinate of cell after full row
    y += sizey;
  }
}

bool Grid::checkPoint(double x, double y) {
  // Do grid boundary check, first do double check to solve for negative numbers
  double i = (x - xmin) / sizex;
  if (i < 0) return false;
  double j = (y - ymin) / sizey;
  if (j < 0) return false;

  int ix = (int)i;
  if (ix >= cellsx) return false;
  int iy = (int)j;
  if (iy >= cellsy) return false;

  //// get cell indices
  //int ix = (int)((x - xmin) / sizex);
  //int iy = (int)((y - ymin) / sizey);

  // get cell color
  int col = cells[ix][iy]->color;

  int crossings = 0;
  if (col != CSINGULAR) {
    // calculate crossings
    for (PolyEdge* e : cells[ix][iy]->edges) {
      /// TESTING
      //double x1 = xmin + (sizex * (ix));
      //double y1 = ymin + (sizey * (iy));
      //double x2 = xmin + (sizex * (ix + 1));
      //double y2 = ymin + (sizey * (iy + 1));
      //std::string wkt = "POLYGON((" 
      //  + std::to_string(x1) + " " + std::to_string(y1) 
      //  + ", " 
      //  + std::to_string(x1) + " " + std::to_string(y2) 
      //  + ", "
      //  + std::to_string(x2) + " " + std::to_string(y2)
      //  + ", "
      //  + std::to_string(x2) + " " + std::to_string(y1)
      //  + ", "
      //  + std::to_string(x1) + " " + std::to_string(y1)
      //  + "))";
      /// TESTING
      Point2* r1 = new Point2(ix, iy);
      Point2* r2 = new Point2(x, y);
      if (RayLineIntersection(r1, r2, e) == DO_INTERSECT) {
        crossings++;
      }
      delete r1, r2;
    }
  }
  else {
    // TODO: find a genious way to select the closes not SINGULAR cell
    int i2 = ix;
    int sign = 1;

    while (cells[i2][iy]->color == CSINGULAR) {
      if (i2 >= cellsx) {
        // when end of row is reached start reverse direction
        i2 = ix;
        sign = -1;
        if (ix == 0) {
          throw std::exception("No non-singular cells found in row, adjust code!");
        }
      }
      if (ix != 0 && i2 == 0) {
        throw std::exception("No non-singular cells found in row, adjust code!");
      }
      i2 = ix + sign;
    }

    // set color to first non-singular cell
    col = cells[i2][iy]->color;

    // create a set with unique edges add edges of current cell and previous cell
    // skip previous if cell is first in row
    std::set<PolyEdge*> edges;
    for (int ii = 0; ii <= sign*(i2 - ix); ii+=sign) {
      edges.insert(cells[ix + ii * sign][iy]->edges.begin(), cells[ix + ii * sign][iy]->edges.end());
    }

    double xc, yc;
    Point2 *r1, *r2;
    // Iterate edges of cell between first and last
    for (PolyEdge* e : edges) {
      //getEdgeToCenter
      xc = xmin + (sizex * (ix + 0.5));
      yc = ymin + (sizey * (iy + 0.5));
      r1 = new Point2(x, y);
      r2 = new Point2(xc, yc);
      if (RayLineIntersection(r1, r2, e) == DO_INTERSECT) {
        crossings++;
      }
      delete r1, r2;
    }
  }
  // equal crossings gives same color for cell
  if (crossings % 2 == 0) {
    if (col == CBLACK) {
      return false;
    }
    else if (col == CWHITE) {
      return true;
    }
  }
  else {
    // inverse color
    if (col == CBLACK) {
      return true;
    }
    else if (col == CWHITE) {
      return false;
    }
  }
  return false;
}

std::set<PolyEdge*> Grid::getEdges(double x, double y, double radius) {
  std::set<PolyEdge*> edges = std::set<PolyEdge*>();
  
  // calculate box for point radius
  double rxmin = x - radius;
  double rxmax = x + radius;
  double rymin = y - radius;
  double rymax = y + radius;

  // get cells that are within radius of the point
  int xstart = rxmin > xmin ? (rxmin - xmin) / sizex : 0;
  int xend = rxmax < xmax ? (rxmax - xmin) / sizex : cellsx - 1;
  int ystart = y > ymin ? (rymin - ymin) / sizey : 0;
  int yend = y < ymax ? (rymax - ymin) / sizey : cellsy - 1;

  for (int ix = xstart; ix <= xend; ix++) {
    for (int iy = ystart; iy <= yend; iy++) {
      edges.insert(cells[ix][iy]->edges.begin(), cells[ix][iy]->edges.end());
    }
  }
  return edges;
}

std::set<Point2*> Grid::getVertices(double x, double y, double radius) {
  std::set<Point2*> vertices = std::set<Point2*>();

  // calculate box for point radius
  double rxmin = x - radius;
  double rxmax = x + radius;
  double rymin = y - radius;
  double rymax = y + radius;

  // get cells that are within radius of the point
  int xstart = rxmin > xmin ? (rxmin - xmin) / sizex : 0;
  int xend = rxmax < xmax ? (rxmax - xmin) / sizex : cellsx - 1;
  int ystart = y > ymin ? (rymin - ymin) / sizey : 0;
  int yend = y < ymax ? (rymax - ymin) / sizey : cellsy - 1;

  for (int ix = xstart; ix <= xend; ix++) {
    for (int iy = ystart; iy <= yend; iy++) {
      for (PolyEdge* e : edges) {
        vertices.insert(new Point2(e->v1->x(), e->v1->y()));
        vertices.insert(new Point2(e->v2->x(), e->v2->y()));
      }
    }
  }
  return vertices;
}

bool Grid::sqr_distance(double x, double y, double radius) {
  // calculate box for point radius
  double rxmin = x - radius;
  double rxmax = x + radius;
  double rymin = y - radius;
  double rymax = y + radius;

  // get cells that are within radius of the point
  int xstart = rxmin > xmin ? (rxmin - xmin) / sizex : 0;
  int xend = rxmax < xmax ? (rxmax - xmin) / sizex : cellsx - 1;
  int ystart = rymin > ymin ? (rymin - ymin) / sizey : 0;
  int yend = rymax < ymax ? (rymax - ymin) / sizey : cellsy - 1;

  // TODO: now we do not take into account the vertices are double
  // maybe add the edges to a set and test?
  for (int ix = xstart; ix <= xend; ix++) {
    for (int iy = ystart; iy <= yend; iy++) {
      for (PolyEdge* e : cells[ix][iy]->edges) {
        double dx = x - e->v1->x();
        double dy = y - e->v1->y();
        if (dx * dx + dy * dy <= radius) {
          return true;
        }
      }
    }
  }
  return false;
}

Grid::~Grid() {
  // delete edges
  for (int i = 0; i < edges.size(); i++) {
    delete edges[i];
  }

  // delete cells
  for (int ix = 0; ix < cellsx; ix++) {
    for (int iy = 0; iy < cellsy; iy++) {
      delete cells[ix][iy];
    }
  }

  for (int i = 0; i < cellsx; i++)
    delete[] cells[i];

  delete[] cells;
}
