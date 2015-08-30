#ifndef __3DFIER__Definitions__
#define __3DFIER__Definitions__


#include <iostream>
#include <fstream>  
#include <vector>
#include <map>

#include <gdal/ogrsf_frmts.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bg  = boost::geometry;
namespace bgi = boost::geometry::index;

#include <liblas/liblas.hpp>

typedef bg::model::d2::point_xy<double> Point2d;
typedef bg::model::point<double, 3, bg::cs::cartesian> Point3d;
typedef bg::model::polygon<Point2d, true, true> Polygon2d; //-- cw, first=last

typedef bg::model::box<Point2d> Box;


typedef enum
{
  HORIZONTAL_AVG       = 0, //-- LOD1 buildings
  HORIZONTAL_MEDIAN    = 1, //-- LOD1 buildings
  BOUNDARY_3D          = 2, //-- water, roads: no points added, only 3D boundary
  TIN_ALL_POINTS       = 3,
  TIN_SIMPLIFIED       = 4,
} ExtrusionType;

#endif
