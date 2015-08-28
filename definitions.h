#ifndef __3DFIER__Definitions__
#define __3DFIER__Definitions__


#include <iostream>
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

#include <pdal/PointView.hpp>
#include <pdal/BufferReader.hpp>
#include <pdal/Pointtable.hpp>
#include <pdal/Dimension.hpp>
#include <pdal/Options.hpp>
#include <pdal/LasReader.hpp>

typedef bg::model::d2::point_xy<double> Point2d;
typedef bg::model::point<double, 3, bg::cs::cartesian> Point3d;
typedef bg::model::polygon<Point2d, true, true> Polygon2d; //-- cw, first=last
typedef bg::model::box<Point2d> Box;
typedef std::pair<Box, std::string> Value;

#endif
