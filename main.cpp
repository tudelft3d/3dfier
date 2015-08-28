

#include <iostream>
#include <vector>

#include <gdal/ogrsf_frmts.h>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>

#include <pdal/PointView.hpp>
#include <pdal/BufferReader.hpp>
#include <pdal/Pointtable.hpp>
#include <pdal/Dimension.hpp>
#include <pdal/Options.hpp>
#include <pdal/LasReader.hpp>


namespace bg  = boost::geometry;
namespace bgi = boost::geometry::index;


typedef bg::model::d2::point_xy<double> Point;
typedef bg::model::polygon<Point, true, true> Polygon; //-- cw, first=last
typedef bg::model::box<Point> Box;
typedef std::pair<Box, std::string> Value;

//----------
bool readlas(std::string ifile);
bool do_top10(std::vector<std::pair< Polygon*, std::string> >& lsPolys);
bool do_shp(std::vector<std::pair< Polygon*, std::string> >& lsPolys);
bool indextests(std::vector<std::pair< Polygon*, std::string> >& lsPolys);
bool getOGRFeatures(std::string file, std::vector<std::string> &layers, std::vector<OGRFeature*> &lsOGRFeatures);
//----------



int main(int argc, const char * argv[]) {
  OGRRegisterAll();
  std::vector<std::pair< Polygon*, std::string> > lsPolys;

  readlas("/Users/hugo/data/ahn2/g37en2.laz");
//  do_top10(lsPolys);
//  do_shp(lsPolys);
//  indextests(lsPolys);
  std::cout << "done." << std::endl;
  return 1;
}


bool readlas(std::string ifile) {
  pdal::Options options;
  options.add("filename", ifile);
  pdal::PointTable table;
  pdal::LasReader reader;
  reader.setOptions(options);
  reader.prepare(table);
  pdal::PointViewSet viewSet = reader.execute(table);
  pdal::PointViewPtr view = *viewSet.begin();
  
  for (int i = 0; i < view->size(); i++){
    double x = view->getFieldAs<double>(pdal::Dimension::Id::X, i);
    double y = view->getFieldAs<double>(pdal::Dimension::Id::Y, i);
    double z = view->getFieldAs<double>(pdal::Dimension::Id::Z, i);
    std::cout << x << ", " << y << ", " << z << std::endl;
  }
  return true;
}



bool do_top10(std::vector<std::pair< Polygon*, std::string> >& lsPolys) {
  // std::string ifile = "/Users/hugo/data/top10nl/TOP10NL_37O.gml";
  std::string ifile = "/Users/hugo/temp/000/TOP10NL_37W_00.gml";
  std::cout << "Reading input dataset: " << ifile << std::endl;

  std::vector<std::string> layers;
  layers.push_back("Terrein");
  layers.push_back("Waterdeel");
  layers.push_back("Wegdeel");
  layers.push_back("Gebouw");
  layers.push_back("Gebouw_Vlak");

  std::vector<OGRFeature*> lsOGRFeatures;
  if (getOGRFeatures(ifile, layers, lsOGRFeatures) == false)
    return false;
  int i = 0;
  for (std::vector<OGRFeature*>::iterator f = lsOGRFeatures.begin() ; f != lsOGRFeatures.end(); f++) {
    if (i % 1000 == 0)
      std::cout << "\t#" << i << std::endl;
    i++;
    switch((*f)->GetGeometryRef()->getGeometryType()) {
      case wkbPolygon:
      case wkbPolygon25D: {
        Polygon* p = new Polygon();
        char *output_wkt;
        (*f)->GetGeometryRef()->exportToWkt(&output_wkt);
        bg::read_wkt(output_wkt, *p);
        std::pair<Polygon*, std::string> tmp(p, (*f)->GetFieldAsString("identificatie"));
        lsPolys.push_back(tmp);
        break;
      }
      case wkbMultiPolygon:
      case wkbMultiPolygon25D: {
        std::cout << "Multi-fudgin'-Polygons: TODO" << std::endl;
        break;
      }
      default:
        std::cerr << "\tFeature #" << (*f)->GetFID() << ": unsupported type (";
        std::cerr << "). Skipped." << std::endl;
        continue;
        break;
    }
  }
  std::cout << "# of polygons: " << lsPolys.size() << std::endl;
  return true;
}


bool do_shp(std::vector<std::pair< Polygon*, std::string> >& lsPolys) {
  std::string ifile = "/Users/hugo/Dropbox/data/pprepair/extent/somepolygons2.shp";
  std::cout << "Reading input dataset: " << ifile << std::endl;
  std::vector<std::string> layers;
  layers.push_back("somepolygons2");
  std::vector<OGRFeature*> lsOGRFeatures;
  if (getOGRFeatures(ifile, layers, lsOGRFeatures) == false)
    return false;
  int i = 0;
  for (std::vector<OGRFeature*>::iterator f = lsOGRFeatures.begin() ; f != lsOGRFeatures.end(); f++) {
    if (i % 1000 == 0)
      std::cout << "\t#" << i << std::endl;
    i++;
    switch((*f)->GetGeometryRef()->getGeometryType()) {
      case wkbPolygon:
      case wkbPolygon25D: {
        Polygon* p = new Polygon();
        char *output_wkt;
        (*f)->GetGeometryRef()->exportToWkt(&output_wkt);
        bg::read_wkt(output_wkt, *p);
        //        std::cout << (bg::is_valid(*p) ? "yes" : "no") << std::endl;
        std::pair<Polygon*, std::string> tmp(p, "test");
        lsPolys.push_back(tmp);
        break;
      }
      case wkbMultiPolygon:
      case wkbMultiPolygon25D: {
        std::cout << "Multi-fudgin'-Polygons: TODO" << std::endl;
        break;
      }
      default:
        std::cerr << "\tFeature #" << (*f)->GetFID() << ": unsupported type (";
        std::cerr << "). Skipped." << std::endl;
        continue;
        break;
    }
  }
  std::cout << lsPolys.size() << std::endl;
  return true;
}



bool indextests(std::vector<std::pair< Polygon*, std::string> >& lsPolys) {
  // TODO : why 16 here? I'm clueless about that param, that's the default
  // http://www.boost.org/doc/libs/1_58_0/libs/geometry/doc/html/geometry/spatial_indexes/creation_and_modification.html
  std::cout << "Constructing the R-tree...";
  bgi::rtree< Value, bgi::rstar<16> > rtree;
  for (std::vector<std::pair< Polygon*, std::string> >::iterator it = lsPolys.begin() ; it != lsPolys.end(); it++) {
    Box b = bg::return_envelope<Box>(*(it->first));
    rtree.insert(std::make_pair(b, it->second));
  }
  std::cout << " done." << std::endl;

  std::vector<Value> re;
  Point queryp(74659.1,447682.4);
//  Point queryp(3965657,3238869);
  Point minp(bg::get<0>(queryp) - 1.0, bg::get<1>(queryp) - 1.0);
  Point maxp(bg::get<0>(queryp) + 1.0, bg::get<1>(queryp) + 1.0);
  Box querybox(minp, maxp);
  rtree.query(bgi::intersects(querybox), std::back_inserter(re));
  std::cout << "query result:" << std::endl;
  std::cout << re.size() << std::endl;
  for (auto& v : re) {
    std::cout << "---\npolygon #" << v.second << std::endl;
//    std::cout << std::setprecision(2) << std::fixed << bg::wkt(*(lsPolys[v.second])) << std::endl;
  }
  return true;
}


bool getOGRFeatures(std::string file, std::vector<std::string> &layers, std::vector<OGRFeature*> &lsOGRFeatures) {
  OGRDataSource *dataSource = OGRSFDriverRegistrar::Open(file.c_str(), false);
  if (dataSource == NULL) {
    std::cerr << "Error: Could not open file." << std::endl;
    return false;
  }
  for (std::string layer : layers) {
    OGRLayer *dataLayer = dataSource->GetLayerByName(layer.c_str());
    if (dataLayer == NULL)
      continue;
    dataLayer->ResetReading();
    unsigned int numberOfPolygons = dataLayer->GetFeatureCount(true);
    std::cout << "\tLayer: " << dataLayer->GetName() << std::endl;
    std::cout << "\t(" << numberOfPolygons << " features)" << std::endl;
    OGRFeature *feature;
    while ((feature = dataLayer->GetNextFeature()) != NULL) {
      switch(feature->GetGeometryRef()->getGeometryType()) {
        case wkbPolygon:
        case wkbPolygon25D:
        case wkbMultiPolygon:
        case wkbMultiPolygon25D:{
          lsOGRFeatures.push_back(feature->Clone());
          break;
        }
        default: {
          continue;
        }
      }
    }
  }
  // Free OGR data source
  OGRDataSource::DestroyDataSource(dataSource);
  return true;
}


