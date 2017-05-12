#ifndef __3DFIER__Definitions__
#define __3DFIER__Definitions__

#include <iostream>
#include <iomanip>
#include <cmath>
#include <fstream>  
#include <vector>
#include <set>
#include <map>
#include <unordered_map>

#include <liblas/liblas.hpp>
#include <ogrsf_frmts.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>
#include "boost/filesystem.hpp"
#include <boost/filesystem/operations.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
typedef bg::model::d2::point_xy<double> Point2;
typedef bg::model::segment<Point2> Segment2;
typedef bg::model::linestring<Point2> Linestring2;
typedef bg::model::polygon<Point2, true, false> Polygon2; //-- cw, first!=last
typedef bg::model::ring<Point2, true, false> Ring2; //-- cw, first!=last
typedef bg::model::box<Point2> Box2;
typedef bg::model::point<double, 3, bg::cs::cartesian> Point3;

typedef struct Segment {
  int v0;
  int v1;
} Segment;

typedef struct Triangle {
  int v0;
  int v1;
  int v2;
} Triangle;

typedef struct PolygonFile {
  std::string filename;
  std::string idfield;
  std::string heightfield;
  bool handle_multiple_heights;
  std::vector< std::pair<std::string, std::string> > layers;
} PolygonFile;

typedef struct PointFile {
  std::string filename;
  std::vector<int> lasomits;
  int thinning = 0;
} PointFile;

typedef enum {
   BUILDING   = 0,
   WATER      = 1,
   BRIDGE     = 2,
   ROAD       = 3,
   TERRAIN    = 4,
   FOREST     = 5,
   SEPARATION = 6
} TopoClass;

typedef enum {
  LAS_UNKNOWN      =  0,
  LAS_UNCLASSIFIED =  1,
  LAS_GROUND       =  2,
  LAS_BUILDING     =  6,
  LAS_WATER        =  9,
  LAS_BRIDGE       =  26
} LAS14Class;

#endif
