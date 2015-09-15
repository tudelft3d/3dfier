
#include "Map3d.h"
#include "input.h"



Map3d::Map3d() {
  OGRRegisterAll();
}


Map3d::~Map3d() {
  // TODO : destructor Map3d
  _lsPolys.clear();
}


std::string Map3d::get_citygml() {
  std::stringstream ss;
  ss << get_xml_header();
  ss << get_citygml_namespaces();
  ss << "<gml:name>my3dmap</gml:name>";
  // TODO : get bbox of dataset
  for (auto& p3 : _lsPolys) {
    ss << p3->get_3d_citygml();
  }
  ss << "</CityModel>";
  return ss.str();
}



unsigned long Map3d::get_num_polygons() {
  return _lsPolys.size();
}


const std::vector<Polygon3d*>& Map3d::get_polygons3d() {
  return _lsPolys;
}


Polygon3d* Map3d::add_point(double x, double y, double z, Polygon3d* trythisone) {
  Point2d p(x, y);
  if (trythisone != NULL) {
    if (bg::within(p, *(trythisone->get_polygon2d())) == true) {
      trythisone->add_elevation_point(x, y, z);
      return trythisone;
    }
  }
  std::vector<PairIndexed> re;
  Point2d minp(x - 0.5, y - 0.5);
  Point2d maxp(x + 0.5, y + 0.5);
  Box querybox(minp, maxp);
  _rtree.query(bgi::intersects(querybox), std::back_inserter(re));
  for (auto& v : re) {
    Polygon3d* pgn3 = v.second;
    if (bg::within(p, *(pgn3->get_polygon2d())) == true) {
      pgn3->add_elevation_point(x, y, z);
      return pgn3;
    }
  }
  return NULL;
}


bool Map3d::construct_rtree() {
  std::clog << "Constructing the R-tree...";
  for (auto p: _lsPolys) 
    _rtree.insert(std::make_pair(p->get_bbox2d(), p));
  std::clog << " done." << std::endl;
  return true;
}


bool Map3d::add_polygons_file(std::string ifile, std::string idfield, std::vector< std::pair<std::string, std::string> > &layers) {
  std::clog << "Reading input dataset: " << ifile << std::endl;
  OGRDataSource *dataSource = OGRSFDriverRegistrar::Open(ifile.c_str(), false);
  if (dataSource == NULL) {
    std::cerr << "\tERROR: could not open file, skipping it." << std::endl;
    return false;
  }
  this->extract_and_add_polygon(dataSource, idfield, layers);
  OGRDataSource::DestroyDataSource(dataSource);
  return true;
}
 

bool Map3d::extract_and_add_polygon(OGRDataSource *dataSource, std::string idfield, std::vector< std::pair<std::string, std::string> > &layers) {
 for (auto l : layers) {
    OGRLayer *dataLayer = dataSource->GetLayerByName((l.first).c_str());
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
          bg::unique(*p2); //-- remove duplicate vertices
          std::string t = l.second.substr(0, l.second.find_first_of("-"));
          if (t == "BLOCK") {
            Polygon3dBlock* p3 = new Polygon3dBlock(p2, f->GetFieldAsString(idfield.c_str()), l.second);
            _lsPolys.push_back(p3);
          }
          else if (t == "BOUNDARY3D")
            std::cout << "BOUNDARY3D NOT IMPLEMENTED YET" << std::endl;
          else if (t == "TIN")
            std::cout << "TIN NOT IMPLEMENTED YET" << std::endl;
          break;
        }
        default: {
          continue;
        }
      }
    }
  }
  return true;
}

bool Map3d::add_polygons_file(std::string ifile, std::string idfield, std::string lifttype) {
  std::clog << "Reading input dataset: " << ifile << std::endl;
  OGRDataSource *dataSource = OGRSFDriverRegistrar::Open(ifile.c_str(), false);
  if (dataSource == NULL) {
    std::cerr << "\tERROR: could not open file, skipping it." << std::endl;
    return false;
  }
  std::vector< std::pair<std::string, std::string> > layers;
  int numberOfLayers = dataSource->GetLayerCount();
  for (int currentLayer = 0; currentLayer < numberOfLayers; currentLayer++) {
    OGRLayer *dataLayer = dataSource->GetLayer(currentLayer);
    std::pair<std::string, std::string> p(dataLayer->GetName(), lifttype);
    layers.push_back(p);
  }
  this->extract_and_add_polygon(dataSource, idfield, layers);
  OGRDataSource::DestroyDataSource(dataSource);
  return true;
}


bool Map3d::add_las_file(std::string ifile, int skip) {
  std::clog << "Reading LAS/LAZ file: " << ifile << std::endl;
  if (skip != 0)
    std::clog << "(only reading every " << skip << "th points)" << std::endl;
  std::ifstream ifs;
  ifs.open(ifile.c_str(), std::ios::in | std::ios::binary);
  if (ifs.is_open() == false) {
    std::cerr << "\tERROR: could not open file, skipping it." << std::endl;
    return false;
  }
  liblas::ReaderFactory f;
  liblas::Reader reader = f.CreateWithStream(ifs);
  liblas::Header const& header = reader.GetHeader();
  std::clog << "\t(" << header.GetPointRecordsCount() << " points)" << std::endl;
  int i = 0;
  Polygon3d* lastone = NULL;
  while (reader.ReadNextPoint()) {
    liblas::Point const& p = reader.GetPoint();
    if (skip != 0) {
      if (i %  skip == 0)
        lastone = this->add_point(p.GetX(), p.GetY(), p.GetZ(), lastone);
    }
    else {
      lastone = this->add_point(p.GetX(), p.GetY(), p.GetZ(), lastone);
    }
    if (i % 100000 == 0) 
      printProgressBar(100 * (i / double(header.GetPointRecordsCount())));
    i++;
  }
  std::clog << "done" << std::endl;
  return true;
}

