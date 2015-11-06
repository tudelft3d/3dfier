#include "Map3d.h"
#include "io.h"


Map3d::Map3d() {
  OGRRegisterAll();
  _building_include_floor = false;
  _building_heightref_roof = "median";
  _building_heightref_floor = "percentile-05";
  _building_triangulate = true;
  _terrain_simplification = 0;
  _vegetation_simplification = 0;
  // _water_ = 0;
  _radius_vertex_elevation = 2.0;
}


Map3d::~Map3d() {
  // TODO : destructor Map3d
  _lsPolys.clear();
}

void Map3d::set_building_heightref_roof(std::string h) {
  _building_heightref_roof = h;  
}

void Map3d::set_building_heightref_floor(std::string h) {
  _building_heightref_floor = h;  
}


void Map3d::set_radius_vertex_elevation(float radius) {
  _radius_vertex_elevation = radius;  
}


void Map3d::set_building_include_floor(bool include) {
  _building_include_floor = include;
}

void Map3d::set_building_triangulate(bool triangulate) {
  _building_triangulate = triangulate;
}

void Map3d::set_terrain_simplification(int simplification) {
  _terrain_simplification = simplification;
}

void Map3d::set_vegetation_simplification(int simplification) {
  _vegetation_simplification = simplification;
}

void Map3d::set_water_heightref(std::string h) {
  _water_heightref = h;  
}

void Map3d::set_road_heightref(std::string h) {
  _road_heightref = h;  
}

std::string Map3d::get_citygml() {
  std::stringstream ss;
  ss << get_xml_header();
  ss << get_citygml_namespaces();
  ss << "<gml:name>my3dmap</gml:name>";
  for (auto& p3 : _lsPolys) {
    ss << p3->get_citygml();
  }
  ss << "</CityModel>";
  return ss.str();
}

std::string Map3d::get_csv_buildings() {
  std::stringstream ss;
  ss << "id;roof;floor" << std::endl;
  for (auto& p3 : _lsPolys) {
    Building* b = dynamic_cast<Building*>(p3);
    if (b != nullptr)
      ss << b->get_csv();
  }
  return ss.str();
}

std::string Map3d::get_obj() {
  std::vector<int> offsets;
  offsets.push_back(0);
  std::stringstream ss;
  ss << "mtllib ./3dfier.mtl" << std::endl;
  for (auto& p3 : _lsPolys) {
    ss << p3->get_obj_v();
    offsets.push_back(p3->get_number_vertices());
  }
  int i = 0;
  int offset = 0;
  for (auto& p3 : _lsPolys) {
    ss << "o " << p3->get_id() << std::endl;
    offset += offsets[i++];
    ss << p3->get_obj_f(offset);
    if (_building_include_floor == true) {  
      Building* b = dynamic_cast<Building*>(p3);
      if (b != nullptr)
        ss << b->get_obj_f_floor(offset);
    }
 }
 return ss.str();
}


unsigned long Map3d::get_num_polygons() {
  return _lsPolys.size();
}


const std::vector<TopoFeature*>& Map3d::get_polygons3d() {
  return _lsPolys;
}


TopoFeature* Map3d::add_elevation_point(double x, double y, double z, TopoFeature* trythisone) {
  Point2 p(x, y);
  if (trythisone != NULL) {
    if (bg::distance(p, *(trythisone->get_Polygon2())) < _radius_vertex_elevation) {
      trythisone->add_elevation_point(x, y, z, _radius_vertex_elevation);
      return trythisone;
    }
  }
  std::vector<PairIndexed> re;
  Point2 minp(x - 0.1, y - 0.1);
  Point2 maxp(x + 0.1, y + 0.1);
  Box2 querybox(minp, maxp);
  _rtree.query(bgi::intersects(querybox), std::back_inserter(re));
  for (auto& v : re) {
    TopoFeature* pgn = v.second;
    if (bg::distance(p, *(pgn->get_Polygon2())) < _radius_vertex_elevation) {     
      pgn->add_elevation_point(x, y, z, _radius_vertex_elevation);
      return pgn;
    }
  }
  return NULL;
}


bool Map3d::threeDfy() {
  for (auto& p3 : _lsPolys)
    p3->threeDfy();
  return true;
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
  bool wentgood = this->extract_and_add_polygon(dataSource, idfield, layers);
  OGRDataSource::DestroyDataSource(dataSource);
  return wentgood;
}


bool Map3d::add_polygons_file(std::string ifile, std::string idfield, std::string lifting) {
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
    std::pair<std::string, std::string> p(dataLayer->GetName(), lifting);
    layers.push_back(p);
  }
  bool wentgood = this->extract_and_add_polygon(dataSource, idfield, layers);
  OGRDataSource::DestroyDataSource(dataSource);
  return wentgood;
}


bool Map3d::extract_and_add_polygon(OGRDataSource *dataSource, std::string idfield, std::vector< std::pair<std::string, std::string> > &layers) {
 bool wentgood = true;
 for (auto l : layers) {
    OGRLayer *dataLayer = dataSource->GetLayerByName((l.first).c_str());
    if (dataLayer == NULL) {
      continue;
    }
    if (dataLayer->FindFieldIndex(idfield.c_str(), false) == -1) {
      std::cerr << "ERROR: field '" << idfield << "' not found." << std::endl;
      wentgood = false;
      continue;
    }
    dataLayer->ResetReading();
    unsigned int numberOfPolygons = dataLayer->GetFeatureCount(true);
    std::clog << "\tLayer: " << dataLayer->GetName() << std::endl;
    std::clog << "\t(" << numberOfPolygons << " features --> " << l.second << ")" << std::endl;
    OGRFeature *f;
    while ((f = dataLayer->GetNextFeature()) != NULL) {
      switch(f->GetGeometryRef()->getGeometryType()) {
        case wkbPolygon:
        case wkbMultiPolygon: 
        case wkbMultiPolygon25D:
        case wkbPolygon25D: {
          Polygon2* p2 = new Polygon2();
          // TODO : WKT surely not best/fastest way, to change. Or is it?
          char *output_wkt;
          f->GetGeometryRef()->flattenTo2D();
          f->GetGeometryRef()->exportToWkt(&output_wkt);
          bg::read_wkt(output_wkt, *p2);
          bg::unique(*p2); //-- remove duplicate vertices
          if (l.second == "Building") {
            Building* p3 = new Building(p2, f->GetFieldAsString(idfield.c_str()), _building_heightref_roof, _building_heightref_floor);
            _lsPolys.push_back(p3);
          }
          else if (l.second == "Terrain") {
            Terrain* p3 = new Terrain(p2, f->GetFieldAsString(idfield.c_str()), this->_terrain_simplification);
            _lsPolys.push_back(p3);
          }
          else if (l.second == "Vegetation") {
            Vegetation* p3 = new Vegetation(p2, f->GetFieldAsString(idfield.c_str()), this->_terrain_simplification);
            _lsPolys.push_back(p3);
          }
          else if (l.second == "Water") {
            Water* p3 = new Water(p2, f->GetFieldAsString(idfield.c_str()), this->_water_heightref);
            _lsPolys.push_back(p3);
          }
          else if (l.second == "Road") {
            Road* p3 = new Road(p2, f->GetFieldAsString(idfield.c_str()), this->_road_heightref);
            _lsPolys.push_back(p3);
          }          
          break;
        }
        default: {
          continue;
        }
      }
    }
  }
  return wentgood;
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
  TopoFeature* lastone = NULL;
  while (reader.ReadNextPoint()) {
    liblas::Point const& p = reader.GetPoint();
    if (skip != 0) {
      if (i % skip == 0)
        lastone = this->add_elevation_point(p.GetX(), p.GetY(), p.GetZ(), lastone);
    }
    else {
      lastone = this->add_elevation_point(p.GetX(), p.GetY(), p.GetZ(), lastone);
    }
    if (i % 100000 == 0) 
      printProgressBar(100 * (i / double(header.GetPointRecordsCount())));
    i++;
  }
  std::clog << "done" << std::endl;
  ifs.close();
  return true;
}


void Map3d::stitch_lifted_features() {
  std::cout << "==========" << std::endl;
  TopoFeature* f = _lsPolys[319];
  std::cout << "#" << f->get_counter() << " : " << f->get_id() << std::endl;
  Polygon2* pgn = f->get_Polygon2();
  Ring2 oring = bg::exterior_ring(*pgn);
  std::vector<PairIndexed> re;
  _rtree.query(bgi::intersects(f->get_bbox2d()), std::back_inserter(re));
  std::cout << "r-tree intersects: " << re.size() << std::endl;
  for (auto& b : re) {
    TopoFeature* adj = b.second;
    // TODO: do not do twice the work, only low_id --> high_id should be stitched
    // if ( (adj->get_counter() > f->get_counter()) && (bg::touches(*pgn, *(adj->get_Polygon2())) == true) ) {
    if ( (bg::touches(*pgn, *(adj->get_Polygon2())) == true) ) {
      std::cout << "---" << std::endl;
      for (int i = 0; i < (oring.size() - 1); i++) {
        if (polygon2_find_segment(adj->get_Polygon2(), oring[i], oring[i + 1]) == true)
          std::cout << adj->get_id() << std::endl;
      }
      if (polygon2_find_segment(adj->get_Polygon2(), oring.back(), oring.front()) == true)
        std::cout << adj->get_id() << std::endl;
      for (Ring2& iring: bg::interior_rings(*pgn)) {
        for (int i = 0; i < (iring.size() - 1); i++) {
          if (polygon2_find_segment(adj->get_Polygon2(), iring[i], iring[i + 1]) == true)
            std::cout << "IRING" << adj->get_id() << std::endl;
        }
        if (polygon2_find_segment(adj->get_Polygon2(), iring.back(), iring.front()) == true)
          std::cout << "IRING" << adj->get_id() << std::endl;
      }
    }
  }
  std::cout << "==========" << std::endl;
}





