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
  PolyEdge* r = new PolyEdge(-2.5, 2.5, 2.5, 2.5);
  PolyEdge* e = new PolyEdge(0, 0, 0, 10);
  BOOST_CHECK(RayLineIntersection(r, e) == DO_INTERSECT);

  // Ray not intersects line in up direction
  r = new PolyEdge(2.5, 2.5, 7.5, 2.5);
  e = new PolyEdge(0, 0, 0, 10);
  BOOST_CHECK(RayLineIntersection(r, e) == DONT_INTERSECT);

  // Ray intersects line in down direction
  r = new PolyEdge(7.5, 2.5, 12.5, 2.5);
  e = new PolyEdge(10, 10, 10, 0);
  BOOST_CHECK(RayLineIntersection(r, e) == DO_INTERSECT);

  // Ray not intersects line in down direction
  r = new PolyEdge(2.5, 2.5, 7.5, 2.5);
  e = new PolyEdge(10, 10, 10, 0);
  BOOST_CHECK(RayLineIntersection(r, e) == DONT_INTERSECT);

  // Ray not intersects line, lines parallel edge positive direction
  r = new PolyEdge(-2.5, 2.5, 2.5, 2.5);
  e = new PolyEdge(0, 10, 10, 10);
  BOOST_CHECK(RayLineIntersection(r, e) == DONT_INTERSECT);
  
  // Ray not intersects line, lines parallel edge negative direction
  r = new PolyEdge(-2.5, 2.5, 2.5, 2.5);
  e = new PolyEdge(10, 0, 0, 0);
  BOOST_CHECK(RayLineIntersection(r, e) == DONT_INTERSECT);

  // Ray end intersects line in the middle
  r = new PolyEdge(0, 5, 5, 5);
  e = new PolyEdge(10, 10, 0, 0);
  BOOST_CHECK(RayLineIntersection(r, e) == BOUNDARY);

  // Ray end intersects line at boundary
  r = new PolyEdge(5, 10, 10, 10);
  e = new PolyEdge(10, 10, 0, 0);
  BOOST_CHECK(RayLineIntersection(r, e) == BOUNDARY);
}

BOOST_AUTO_TEST_CASE(HorizontalRayLineIntersections) {
  // Ray intersects line in up direction
  PolyEdge* r = new PolyEdge(0, 5, 5, 5);
  PolyEdge* e = new PolyEdge(5, 0, 5, 10);
  BOOST_CHECK(HorizontalRayLineIntersection(r, e) == BOUNDARY);
}

BOOST_AUTO_TEST_CASE(GridPrepare) {
  std::string wkt = "POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0))";
  Polygon2* _p2 = new Polygon2();
  bg::read_wkt(wkt, *_p2);

  Grid* _grid = new Grid(_p2);
  _grid->prepare();

  BOOST_CHECK(_grid->celllimit == 30);
  BOOST_CHECK(_grid->cellsx == 2);
  BOOST_CHECK(_grid->cellsy == 2);
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