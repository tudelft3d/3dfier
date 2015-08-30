
#include "Map3d.h"



Map3d::Map3d() {
  OGRRegisterAll();
}

Map3d::Map3d(std::vector<std::string> allowed_layers) {
  OGRRegisterAll();
  this->add_allowed_layers(allowed_layers);
}

Map3d::~Map3d() {
  // TODO : destructor Map3d
  _lsPolys.clear();
}


bool Map3d::add_polygon3d(Polygon3d* pgn) {
  _lsPolys.push_back(pgn);
  return true;
}


bool Map3d::add_allowed_layer(std::string l) {
  _allowed_layers.push_back(l);
  return true;
}

unsigned long Map3d::get_num_polygons() {
  return _lsPolys.size();
}

bool Map3d::add_allowed_layers(std::vector<std::string> ls) {
  _allowed_layers.insert(_allowed_layers.end(), ls.begin(), ls.end());
  return true;
}


const std::vector<Polygon3d*>& Map3d::get_polygons3d() {
  return _lsPolys;
}


bool Map3d::add_point(double x, double y, double z) {
  std::vector<PairIndexed> re;
  Point2d p(x, y);
  Point2d minp(x - 0.5, y - 0.5);
  Point2d maxp(x + 0.5, y + 0.5);
  Box querybox(minp, maxp);
  _rtree.query(bgi::intersects(querybox), std::back_inserter(re));
  for (auto& v : re) {
    Polygon3d* pgn3 = v.second;
    if (bg::within(p, *(pgn3->get_polygon2d())) == true)
      pgn3->add_elevation_point(x, y, z);
  }
  return true;
}


bool Map3d::construct_rtree() {
  std::clog << "Constructing the R-tree...";
  for (auto p: _lsPolys) 
    _rtree.insert(std::make_pair(p->get_bbox2d(), p));
  std::clog << " done." << std::endl;
  return true;
}


bool Map3d::add_gml_file(std::string ifile, std::string idfield) {
  std::clog << "Reading input dataset: " << ifile << std::endl;
  OGRDataSource *dataSource = OGRSFDriverRegistrar::Open(ifile.c_str(), false);
  if (dataSource == NULL) {
    std::cerr << "Error: Could not open file." << std::endl;
    return false;
  }
  for (std::string l : _allowed_layers) {
    OGRLayer *dataLayer = dataSource->GetLayerByName(l.c_str());
    if (dataLayer == NULL) {
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
          // TODO : type of extrusion should be taken from config file
          Polygon3d_H_AVG* p3 = new Polygon3d_H_AVG(p2, f->GetFieldAsString(idfield.c_str()));
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

     

bool Map3d::add_las_file(std::string ifile) {
  std::clog << "Reading LAS/LAZ file: " << ifile;
  pdal::Options options;
  options.add("filename", ifile);
  pdal::PointTable table;
  pdal::LasReader reader;
  reader.setOptions(options);
  reader.prepare(table);
  pdal::PointViewSet viewSet = reader.execute(table);
  pdal::PointViewPtr view = *viewSet.begin();
  std::clog << " (" << view->size() << " points)";
  for (int i = 0; i < view->size(); i++) {
    this->add_point(view->getFieldAs<double>(pdal::Dimension::Id::X, i),
                    view->getFieldAs<double>(pdal::Dimension::Id::Y, i),
                    view->getFieldAs<double>(pdal::Dimension::Id::Z, i) );
  }
  std::clog << "done" << std::endl;
  return true;
}

