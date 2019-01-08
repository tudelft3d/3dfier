#include "../src/definitions.h"
#include "../src/geomtools.h"

#define BOOST_TEST_MODULE "GridTestModule"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(GridConstructor) {
  std::string wkt = "POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))";
  Polygon2* _p2 = new Polygon2();
  bg::read_wkt(wkt, *_p2);

  Grid* _grid = new Grid(_p2);
  
  //Check default constructor
  //Use BOOST_CHECK for small equal checks - true or false
  BOOST_CHECK(_grid->celllimit == 30);
}

BOOST_AUTO_TEST_CASE(RayLineIntersections) {
  // Ray intersects line in up direction
  Point2* r1 = new Point2(-2.5, 2.5);
  Point2* r2 = new Point2(2.5, 2.5);
  Point2* v1 = new Point2(0, 0);
  Point2* v2 = new Point2(0, 10);
  PolyEdge* e = new PolyEdge(v1, v2, 0, 0, 0);
  BOOST_CHECK(RayLineIntersection(r1,r2, e) == DO_INTERSECT);

  // Ray not intersects line in up direction
  r1 = new Point2(2.5, 2.5);
  r2 = new Point2(7.5, 2.5);
  v1 = new Point2(0, 0);
  v2 = new Point2(0, 10);
  e = new PolyEdge(v1, v2, 0, 0, 0);
  BOOST_CHECK(RayLineIntersection(r1, r2, e) == DONT_INTERSECT);

  // Ray intersects line in down direction
  r1 = new Point2(7.5, 2.5);
  r2 = new Point2(12.5, 2.5);
  v1 = new Point2(10, 10);
  v2 = new Point2(10, 0);
  e = new PolyEdge(v1, v2, 0, 0, 0);
  BOOST_CHECK(RayLineIntersection(r1, r2, e) == DO_INTERSECT);

  // Ray not intersects line in down direction
  r1 = new Point2(2.5, 2.5);
  r2 = new Point2(7.5, 2.5);
  v1 = new Point2(10, 10);
  v2 = new Point2(10, 0);
  e = new PolyEdge(v1, v2, 0, 0, 0);
  BOOST_CHECK(RayLineIntersection(r1, r2, e) == DONT_INTERSECT);

  // Ray not intersects line, lines parallel edge positive direction
   r1 = new Point2(-2.5, 2.5);
   r2 = new Point2(2.5, 2.5);
   v1 = new Point2(0, 10);
   v2 = new Point2(10, 10);
   e = new PolyEdge(v1, v2, 0, 0, 0);
  BOOST_CHECK(RayLineIntersection(r1, r2, e) == DONT_INTERSECT);
  
  // Ray not intersects line, lines parallel edge negative direction
   r1 = new Point2(-2.5, 2.5);
   r2 = new Point2(2.5, 2.5);
   v1 = new Point2(10, 0);
   v2 = new Point2(0, 0);
   e = new PolyEdge(v1, v2, 0, 0, 0);
  BOOST_CHECK(RayLineIntersection(r1, r2, e) == DONT_INTERSECT);
}

BOOST_AUTO_TEST_CASE(HorizontalRayLineIntersections) {
  // Ray intersects line in up direction
  Point2* r1 = new Point2(0, 5);
  Point2* r2 = new Point2(5, 5);
  Point2* v1 = new Point2(5, 0);
  Point2* v2 = new Point2(5, 10);
  PolyEdge* e = new PolyEdge(v1, v2, 0, 0, 0);
  BOOST_CHECK(HorizontalRayLineIntersection(r1, r2, e) == BOUNDARY);

  // Ray end intersects line in the middle
  r1 = new Point2(0, 5);
  r2 = new Point2(5, 5);
  v1 = new Point2(10, 10);
  v2 = new Point2(0, 0);
  e = new PolyEdge(v1, v2, 0, 0, 0);
  BOOST_CHECK(HorizontalRayLineIntersection(r1, r2, e) == BOUNDARY);

  // Ray end intersects line at boundary
  r1 = new Point2(5, 10);
  r2 = new Point2(10, 10);
  v1 = new Point2(10, 10);
  v2 = new Point2(0, 0);
  e = new PolyEdge(v1, v2, 0, 0, 0);
  BOOST_CHECK(HorizontalRayLineIntersection(r1, r2, e) == BOUNDARY);
}

BOOST_AUTO_TEST_CASE(GridPrepare) {
  std::string wkt = "POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))";
  Polygon2* _p2 = new Polygon2();
  bg::read_wkt(wkt, *_p2);

  Grid* _grid = new Grid(_p2);
  _grid->prepare();

  BOOST_CHECK(_grid->celllimit == 30);
  BOOST_CHECK(_grid->cellsx == 6);
  BOOST_CHECK(_grid->cellsy == 6);
}

BOOST_AUTO_TEST_CASE(GridCheckPoint) {
  std::string wkt = "POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))";
  Polygon2* _p2 = new Polygon2();
  bg::read_wkt(wkt, *_p2);

  Grid* _grid = new Grid(_p2);
  _grid->prepare();

  BOOST_CHECK(_grid->checkPoint(1, 1) == true);
  BOOST_CHECK(_grid->checkPoint(-1, -1) == false);
}

BOOST_AUTO_TEST_CASE(GridSingularCell) {
  std::string wkt = "POLYGON ((0 0, 0 10, 10 10, 10 3, 7.55 3, 7.55 0.2, 10 0.2, 10 0, 0 0))";
  Polygon2* _p2 = new Polygon2();
  bg::read_wkt(wkt, *_p2);

  Grid* _grid = new Grid(_p2);
  _grid->prepare();

  BOOST_CHECK(_grid->checkPoint(7, 1) == true);
  BOOST_CHECK(_grid->checkPoint(8, 1) == false);
}

BOOST_AUTO_TEST_CASE(GridPolygonHole) {
  // create Grid from polygon with hole
  BOOST_CHECK(false);
}

BOOST_AUTO_TEST_CASE(GridGetEdges) {
  std::string wkt = "POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))";
  Polygon2* _p2 = new Polygon2();
  bg::read_wkt(wkt, *_p2);

  Grid* _grid = new Grid(_p2);
  _grid->prepare();

  std::set<PolyEdge*> edges = _grid->getEdges(5, 5, 1);
  BOOST_CHECK(edges.size() == 0);
  edges = _grid->getEdges(6, 6, 0.5);
  BOOST_CHECK(edges.size() == 0);
  edges = _grid->getEdges(9, 5, 0.5);
  BOOST_CHECK(edges.size() == 1);
  edges = _grid->getEdges(9, 9, 0.5);
  BOOST_CHECK(edges.size() == 2);
}