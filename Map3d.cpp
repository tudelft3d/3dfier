/*
 Copyright (c) 2015 Hugo Ledoux
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

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
  _radius_vertex_elevation = 1.0;
  _minx = 1e8;
  _miny = 1e8;
}


Map3d::~Map3d() {
  // TODO : destructor Map3d
  _lsFeatures.clear();
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
  for (auto& p3 : _lsFeatures) {
    ss << p3->get_citygml();
  }
  ss << "</CityModel>";
  return ss.str();
}

std::string Map3d::get_csv_buildings() {
  std::stringstream ss;
  ss << "id;roof;floor" << std::endl;
  for (auto& p : _lsFeatures) {
    if (p->get_class() == BUILDING) {
      Building* b = dynamic_cast<Building*>(p);
      // if (b != nullptr)
      ss << b->get_csv();
    }
  }
  return ss.str();
}

std::string Map3d::get_obj() {
  std::vector<int> offsets;
  offsets.push_back(0);
  std::stringstream ss;
  ss << "mtllib ./3dfier.mtl" << std::endl;
  for (auto& p3 : _lsFeatures) {
    ss << p3->get_obj_v();
    offsets.push_back(p3->get_number_vertices());
  }
  int i = 0;
  int offset = 0;
  for (auto& p3 : _lsFeatures) {
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
  return _lsFeatures.size();
}


const std::vector<TopoFeature*>& Map3d::get_polygons3d() {
  return _lsFeatures;
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


bool Map3d::threeDfy(bool triangulate) {
/*
  1. lift
  2. stitch
  3. CDT
*/
  std::clog << "MinX: " << _minx << std::endl;
  std::clog << "MinY: " << _miny << std::endl;
  for (auto& p : _lsFeatures)
    p->lift();
  if (triangulate == true) {
    // this->stitch_lifted_features();
    for (auto& p : _lsFeatures) {
      std::clog << p->get_id() << std::endl;
      p->buildCDT();
    }
  }
  return true;
}


bool Map3d::construct_rtree() {
  std::clog << "Constructing the R-tree...";
  for (auto p: _lsFeatures) 
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
  //-- find minx/miny of all datasets
  // TODO : also find minx/miny for the method above with GML
  OGREnvelope bbox;
  for (auto l : layers) {
    OGRLayer *dataLayer = dataSource->GetLayerByName((l.first).c_str());
    if (dataLayer == NULL) {
      continue;
    }
    dataLayer->GetExtent(&bbox);
    if (bbox.MinX < _minx)
      _minx = bbox.MinX;
    if (bbox.MinY < _miny)
      _miny = bbox.MinY;
  }
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
          char *wkt;
          f->GetGeometryRef()->flattenTo2D();
          f->GetGeometryRef()->exportToWkt(&wkt);

          bg::unique(*p2); //-- remove duplicate vertices
          if (l.second == "Building") {
            Building* p3 = new Building(wkt, f->GetFieldAsString(idfield.c_str()), _building_heightref_roof, _building_heightref_floor);
            _lsFeatures.push_back(p3);
          }
          else if (l.second == "Terrain") {
            Terrain* p3 = new Terrain(wkt, f->GetFieldAsString(idfield.c_str()), this->_terrain_simplification);
            _lsFeatures.push_back(p3);
          }
          else if (l.second == "Vegetation") {
            Vegetation* p3 = new Vegetation(wkt, f->GetFieldAsString(idfield.c_str()), this->_terrain_simplification);
            _lsFeatures.push_back(p3);
          }
          else if (l.second == "Water") {
            Water* p3 = new Water(wkt, f->GetFieldAsString(idfield.c_str()), this->_water_heightref);
            _lsFeatures.push_back(p3);
          }
          else if (l.second == "Road") {
            Road* p3 = new Road(wkt, f->GetFieldAsString(idfield.c_str()), this->_road_heightref);
            _lsFeatures.push_back(p3);
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


//-- http://www.liblas.org/tutorial/cpp.html#applying-filters-to-a-reader-to-extract-specified-classes
bool Map3d::add_las_file(std::string ifile, std::vector<int> lasomits, int skip) {
  std::clog << "Reading LAS/LAZ file: " << ifile << std::endl;
  if (lasomits.empty() == false) {
    std::clog << "\t(omitting LAS classes: ";
    for (int i : lasomits)
      std::clog << i << " ";
    std::clog << ")" << std::endl;
  }
  if (skip != 0)
    std::clog << "\t(only reading every " << skip << "th points)" << std::endl;
  std::ifstream ifs;
  ifs.open(ifile.c_str(), std::ios::in | std::ios::binary);
  if (ifs.is_open() == false) {
    std::cerr << "\tERROR: could not open file, skipping it." << std::endl;
    return false;
  }
  //-- LAS classes to omit
  std::vector<liblas::Classification> liblasomits;
  for (int i : lasomits)
    liblasomits.push_back(liblas::Classification(i));
  //-- read each point 1-by-1
  liblas::ReaderFactory f;
  liblas::Reader reader = f.CreateWithStream(ifs);
  liblas::Header const& header = reader.GetHeader();
  std::clog << "\t(" << header.GetPointRecordsCount() << " points)" << std::endl;
  int i = 0;
  TopoFeature* lastone = NULL;
  while (reader.ReadNextPoint()) {
    liblas::Point const& p = reader.GetPoint();
    if (skip != 0) {
      if (i % skip == 0) {
        if(std::find(liblasomits.begin(), liblasomits.end(), p.GetClassification()) == liblasomits.end())
          lastone = this->add_elevation_point(p.GetX(), p.GetY(), p.GetZ(), lastone);
      }
    }
    else {
      if(std::find(liblasomits.begin(), liblasomits.end(), p.GetClassification()) == liblasomits.end())
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


void Map3d::stitch_water(TopoFeature* f, std::vector<PairIndexed> &re) {
  std::clog << "#" << f->get_counter() << " : " << f->get_id() << std::endl;
  Polygon2* pgn = f->get_Polygon2();
  Ring2 oring = bg::exterior_ring(*pgn);
  for (auto& each : re) {
    TopoFeature* fadj = each.second;
    if (fadj->get_class() == TERRAIN) {
      if ( (bg::touches(*pgn, *(fadj->get_Polygon2())) == true) ) {
        int ia, ib;
        for (int i = 0; i < (oring.size() - 1); i++) {
          if (fadj->has_segment(oring[i], oring[i + 1], ia, ib) == true) {
            float z = f->get_point_elevation(f->has_point2(oring[i]));
            fadj->set_point_elevation(ia, z);
            fadj->set_point_elevation(ib, z);  
          }
        }
        if (fadj->has_segment(oring.back(), oring.front(), ia, ib) == true) {
          float z = f->get_point_elevation(f->has_point2(oring.front()));
          fadj->set_point_elevation(ia, z);
          fadj->set_point_elevation(ib, z);  
        }
      }
    }
  }
}

void Map3d::stitch_road(TopoFeature* f, std::vector<PairIndexed> &re) {
  std::clog << "#" << f->get_counter() << " : " << f->get_id() << std::endl;
  Polygon2* pgn = f->get_Polygon2();
  Ring2 oring = bg::exterior_ring(*pgn);
  for (auto& each : re) {
    TopoFeature* fadj = each.second;
    if (fadj->get_class() == ROAD) {
      // if ( (bg::touches(*pgn, *(fadj->get_Polygon2())) == true) ) {
      if ( (fadj->get_counter() > f->get_counter()) && (bg::touches(*pgn, *(fadj->get_Polygon2())) == true) ) {
        int ia, ib;
        for (int i = 0; i < (oring.size() - 1); i++) {
          if (fadj->has_segment(oring[i], oring[i + 1], ia, ib) == true) {
            int j = f->has_point2(oring[i]);
            float z = f->get_point_elevation(j);
            float zadj = fadj->get_point_elevation(ia);
            f->set_point_elevation(j, (z + zadj) / 2);
            fadj->set_point_elevation(ia, (z + zadj) / 2);
            //--
            j = f->has_point2(oring[i + 1]);
            z = f->get_point_elevation(j);
            zadj = fadj->get_point_elevation(ib);
            f->set_point_elevation(j, (z + zadj) / 2);
            fadj->set_point_elevation(ib, (z + zadj) / 2);
          }
        }
        // if (fadj->has_segment(oring.back(), oring.front(), ia, ib) == true) {
        //   float z = f->get_point_elevation(f->has_point2(oring.front()));
        //   float zadj = fadj->get_point_elevation(ia);
        //   fadj->set_point_elevation(ia, (z + zadj) / 2);
        //   zadj = fadj->get_point_elevation(ib);
        //   fadj->set_point_elevation(ib, (z + zadj) / 2);
        // }
      }
    }
  } 
}


// void Map3d::stitch_lower(TopoFeature* f, TopoFeature* fafj) {
//   std::clog << "#" << f->get_counter() << " : " << f->get_id() << std::endl;
//   Polygon2* pgn = f->get_Polygon2();
//   Ring2 oring = bg::exterior_ring(*pgn);
//   for (auto& each : re) {
//     TopoFeature* fadj = each.second;
//     if (fadj->get_class() == TERRAIN) {
//       if ( (bg::touches(*pgn, *(fadj->get_Polygon2())) == true) ) {
//         int ia, ib;
//         for (int i = 0; i < (oring.size() - 1); i++) {
//           if (fadj->has_segment(oring[i], oring[i + 1], ia, ib) == true) {
//             float z = f->get_point_elevation(f->has_point2(oring[i]));
//             fadj->set_point_elevation(ia, z);
//             fadj->set_point_elevation(ib, z);  
//           }
//         }
//         if (fadj->has_segment(oring.back(), oring.front(), ia, ib) == true) {
//           float z = f->get_point_elevation(f->has_point2(oring.front()));
//           fadj->set_point_elevation(ia, z);
//           fadj->set_point_elevation(ib, z);  
//         }
//       }
//     }
//   }
// }


// void Map3d::stitch_lifted_features_2() {
// /*
//   WATER - TERRAIN
//   WATER - ROAD
// */

//   for (auto& f : _lsFeatures) {
//     if (f->get_class() == WATER) {
//     std::vector<PairIndexed> re;
//     _rtree.query(bgi::intersects(f->get_bbox2d()), std::back_inserter(re));
//     for (auto& each : re) {
//       TopoFeature* fadj = each.second;
  
// }

void Map3d::stitch_one_feature(TopoFeature* f, TopoClass adjclass) {
  std::vector<PairIndexed> re;
  _rtree.query(bgi::intersects(f->get_bbox2d()), std::back_inserter(re));
  for (auto& each : re) {
    // TopoFeature* fadj = each.second;
    if ( each.second->get_class() == adjclass ) {
      std::cout << each.second->get_id() << std::endl;
    }
  }
}

void Map3d::stitch_lifted_features() {
  std::clog << "===== STITCH POLYGONS =====" << std::endl;
//  for (auto& f : _lsFeatures) {
//
//    switch(f->get_class()) {
//      case 1: int x = 0; // initialization
//            std::cout << x << '\n';
//            break;
//    default: // compilation error: jump to default: would enter the scope of 'x'
//             // without initializing it
//             std::cout << "default\n";
//             break;
//}
//
//    if (f->get_class() == WATER)
//      stitch_one_feature(f, TERRAIN);

  std::clog << "===== STITCH POLYGONS =====" << std::endl;
}



// void Map3d::stitch_lifted_features() {
//   std::clog << "===== STITCH POLYGONS =====" << std::endl;
//   for (auto& f : _lsFeatures) {
//     std::vector<PairIndexed> re;
//     _rtree.query(bgi::intersects(f->get_bbox2d()), std::back_inserter(re));
//     for (auto& each : re) {
//       TopoFeature* fadj = each.second;
//       if ( (f->get_class() == WATER) && (fadj->get_class() == TERRAIN) ) {
//         if ( (bg::touches(*(f->get_Polygon2()), *(fadj->get_Polygon2())) == true) ) {
//           int ia, ib;
//           Ring2 oring = bg::exterior_ring(*(f->get_Polygon2()));
//           for (int i = 0; i < (oring.size() - 1); i++) {
//             if (fadj->has_segment(oring[i], oring[i + 1], ia, ib) == true) {
//               float z = f->get_point_elevation(f->has_point2(oring[i]));
//               fadj->set_point_elevation(ia, z);
//               fadj->set_point_elevation(ib, z);  
//             }
//           }
//           if (fadj->has_segment(oring.back(), oring.front(), ia, ib) == true) {
//             float z = f->get_point_elevation(f->has_point2(oring.front()));
//             fadj->set_point_elevation(ia, z);
//             fadj->set_point_elevation(ib, z);  
//           }
//         }
//       }
//     }
//   }
//   std::clog << "===== STITCH POLYGONS =====" << std::endl;
// }

// void Map3d::stitch_lifted_features() {
//   std::clog << "===== STITCH POLYGONS =====" << std::endl;
//   for (auto& f : _lsFeatures) {
//     std::vector<PairIndexed> re;
//     _rtree.query(bgi::intersects(f->get_bbox2d()), std::back_inserter(re));
//     if (f->get_class() == WATER) 
//       this->stitch_water(f, re);
//     if (f->get_class() == ROAD) 
//       this->stitch_road(f, re);
//   }
//   std::clog << "===== STITCH POLYGONS =====" << std::endl;
// }
// for (Ring2& iring: bg::interior_rings(*pgn)) {
//   for (int i = 0; i < (iring.size() - 1); i++) {
//     if (polygon2_find_segment(fadj->get_Polygon2(), iring[i], iring[i + 1]) == true)
//       std::clog << "IRING" << fadj->get_id() << std::endl;
//   }
//   if (polygon2_find_segment(fadj->get_Polygon2(), iring.back(), iring.front()) == true)
//     std::clog << "IRING" << fadj->get_id() << std::endl;
// }





