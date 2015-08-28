
#include "Map3d.h"


Map3d::Map3d() {
  OGRRegisterAll();
}

Map3d::~Map3d() {
  // TODO : destructor Map3d
}


bool Map3d::add_polygon3d(Polygon3d* pgn) {
  _lsPolys.push_back(pgn);
  return true;
}


bool Map3d::add_possible_layer(std::string l) {
  _possible_layers.push_back(l);
  return true;
}

unsigned long Map3d::get_num_polygons() {
  return _lsPolys.size();
}

bool Map3d::add_possible_layers(std::vector<std::string> ls) {
  _possible_layers.insert(_possible_layers.end(), ls.begin(), ls.end());
  return true;
}

bool Map3d::add_point(Point2d* q) {
  std::vector<Value> re;
  Point2d minp(bg::get<0>(q) - 0.5, bg::get<1>(q) - 0.5);
  Point2d maxp(bg::get<0>(q) + 0.5, bg::get<1>(q) + 0.5);
  Box querybox(minp, maxp);
  _rtree.query(bgi::intersects(querybox), std::back_inserter(re));
  for (auto& v : re) {
    std::cout << "---\npolygon #" << v.second << std::endl;
  }
  return true;
}


bool Map3d::add_point(double x, double y, double z) {
  std::vector<Value> re;
  Point2d minp(x - 0.5, y - 0.5);
  Point2d maxp(x + 0.5, y + 0.5);
  Box querybox(minp, maxp);
  _rtree.query(bgi::intersects(querybox), std::back_inserter(re));
  for (auto& v : re) {
    std::cout << "---\npolygon #" << v.second << std::endl;
  }
  return true;
}


bool Map3d::construct_rtree() {
  std::clog << "Constructing the R-tree...";
  for (auto p: _lsPolys) 
    _rtree.insert(std::make_pair(p->get_bbox2d(), p->get_id()));
  std::cout << " done." << std::endl;
  return true;
}


bool Map3d::read_gml_file(std::string ifile, std::string idfield) {
  std::cout << "Reading input dataset: " << ifile << std::endl;
  OGRDataSource *dataSource = OGRSFDriverRegistrar::Open(ifile.c_str(), false);
  if (dataSource == NULL) {
    std::cerr << "Error: Could not open file." << std::endl;
    return false;
  }
  for (std::string l : _possible_layers) {
    OGRLayer *dataLayer = dataSource->GetLayerByName(l.c_str());
    if (dataLayer == NULL) {
      std::clog << "Layer " << l << " not found." << std::endl;
      continue;
    }
    dataLayer->ResetReading();
    unsigned int numberOfPolygons = dataLayer->GetFeatureCount(true);
    std::clog << "\tLayer: " << dataLayer->GetName() << std::endl;
    std::clog << "\t(" << numberOfPolygons << " features)" << std::endl;
    OGRFeature *f;
    while ((f = dataLayer->GetNextFeature()) != NULL) {
      switch(f->GetGeometryRef()->getGeometryType()) {
        case wkbPolygon:
        case wkbPolygon25D:
        case wkbMultiPolygon:
        case wkbMultiPolygon25D:{
          Polygon2d* p2 = new Polygon2d();
          // TODO : WKT surely not best/fastest way, to change
          char *output_wkt;
          f->GetGeometryRef()->exportToWkt(&output_wkt);
          bg::read_wkt(output_wkt, *p2);
          Polygon3d* p3 = new Polygon3d(p2, SIMPLE_AVG, f->GetFieldAsString(idfield.c_str()));
          _lsPolys.push_back(p3);
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

     

