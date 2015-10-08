#ifndef __3DFIER__Definitions__
#define __3DFIER__Definitions__


#include <iostream>
#include <iomanip>
#include <fstream>  
#include <vector>
#include <map>

#include <liblas/liblas.hpp>
#include <ogrsf_frmts.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bg  = boost::geometry;
namespace bgi = boost::geometry::index;
typedef bg::model::d2::point_xy<double> Point2;
typedef bg::model::polygon<Point2, true, true> Polygon2; //-- cw, first=last
typedef bg::model::ring<Point2, true, true> Ring2; //-- cw, first=last
typedef bg::model::box<Point2> Box;
typedef bg::model::point<double, 3, bg::cs::cartesian> Point3;
typedef bg::model::polygon<Point3, true, true> Polygon3;

//-- extrusion types
// BLOCK- {AVG -- MAX -- MIN -- MEDIAN}
// BOUNDARY3D
// TIN- {ALL_POINTS -- 50 -- 10}


typedef struct Segment
{
  int v0;
  int v1;
} Segment;

typedef struct Triangle
{
  int v0;
  int v1;
  int v2;
} Triangle;


#endif
