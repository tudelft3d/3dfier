#ifndef __3DFIER__Definitions__
#define __3DFIER__Definitions__


#include <iostream>
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
typedef bg::model::d2::point_xy<double> Point2d;
typedef bg::model::point<double, 3, bg::cs::cartesian> Point3d;
typedef bg::model::polygon<Point2d, true, true> Polygon2d; //-- cw, first=last
typedef bg::model::ring<Point2d, true, true> Ring2d; //-- cw, first=last
typedef bg::model::box<Point2d> Box;



typedef enum
{
  BLOCK            = 10,
  BOUNDARY3D       = 20, 
  TIN_ALL_POINTS   = 30,
} Lift3DType;

typedef struct Triangle
{
  int v0;
  int v1;
  int v2;
} Triangle;


#endif
