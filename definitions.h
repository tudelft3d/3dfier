#ifndef __3DFIER__Definitions__
#define __3DFIER__Definitions__


#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>  
#include <vector>
#include <set>
#include <map>

#include <liblas/liblas.hpp>
#include <ogrsf_frmts.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bg  = boost::geometry;
namespace bgi = boost::geometry::index;
typedef bg::model::d2::point_xy<double> Point2;
typedef bg::model::linestring<Point2> Linestring2; 
typedef bg::model::polygon<Point2, true, false> Polygon2; //-- cw, first!=last
typedef bg::model::ring<Point2, true, false> Ring2; //-- cw, first!=last
typedef bg::model::box<Point2> Box2;
typedef bg::model::point<double, 3, bg::cs::cartesian> Point3;
typedef bg::model::polygon<Point3, true, false> Polygon3;
typedef bg::model::ring<Point3, true, false> Ring3; //-- cw, first!=last


//-- for Shewchuk's Triangle
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

typedef enum
{
   BUILDING             = 0,
   WATER                = 1,
   BRIDGE               = 2,
   ROAD                 = 3,
   TERRAIN              = 4,
   FOREST               = 5,
} TopoClass;


#endif
