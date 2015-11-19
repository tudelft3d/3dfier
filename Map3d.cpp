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


bool Map3d::threeDfy() {
/*
  1. lift
  2. stitch
  3. CDT
*/
  for (auto& p : _lsFeatures)
    p->lift();
  this->stitch_lifted_features();
  for (auto& p : _lsFeatures)
    p->buildCDT();
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
            _lsFeatures.push_back(p3);
          }
          else if (l.second == "Terrain") {
            Terrain* p3 = new Terrain(p2, f->GetFieldAsString(idfield.c_str()), this->_terrain_simplification);
            _lsFeatures.push_back(p3);
          }
          else if (l.second == "Vegetation") {
            Vegetation* p3 = new Vegetation(p2, f->GetFieldAsString(idfield.c_str()), this->_terrain_simplification);
            _lsFeatures.push_back(p3);
          }
          else if (l.second == "Water") {
            Water* p3 = new Water(p2, f->GetFieldAsString(idfield.c_str()), this->_water_heightref);
            _lsFeatures.push_back(p3);
          }
          else if (l.second == "Road") {
            Road* p3 = new Road(p2, f->GetFieldAsString(idfield.c_str()), this->_road_heightref);
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
  for (auto& f : _lsFeatures) {
    std::vector<PairIndexed> re;
    _rtree.query(bgi::intersects(f->get_bbox2d()), std::back_inserter(re));
//-- 1. store all touching
    std::vector<TopoFeature*> lstouching;
    for (auto& each : re) {
      TopoFeature* fadj = each.second;
      if (bg::touches(*(f->get_Polygon2()), *(fadj->get_Polygon2())) == true) {
        // std::cout << f->get_id() << "-" << f->get_class() << " : " << fadj->get_id() << "-" << fadj->get_class() << std::endl;
        lstouching.push_back(fadj);
      }
    }
//-- 2. build the node-column for each vertex
    Ring2 oring = bg::exterior_ring(*(f->get_Polygon2())); // TODO: iring needs to be implemented too
    for (int i = 0; i < oring.size(); i++) {
      std::vector< std::pair<TopoFeature*, int> > star;  
      bool toprocess = true;
      for (auto& fadj : lstouching) {
        int index = fadj->has_point2(oring[i]);
        if (index != -1)  {
          if (f->get_counter() < fadj->get_counter()) {
            star.push_back(std::make_pair(fadj, index));
          }
          else {
            toprocess = false;
            break;
          }
        }
      }
      if (toprocess == true) {
        this->process_star(f, i, star);
      }
    }

    // break; //-- FIXME: only the first processed, 
  }
  std::clog << "===== STITCH POLYGONS =====" << std::endl;
}


void Map3d::process_star(TopoFeature* f, int pos, std::vector< std::pair<TopoFeature*, int> >& star) {
  float fz = f->get_point_elevation(pos);
//-- WATER  
  if (f->get_class() == WATER) {
    if (star.size() == 1) {
      if (star[0].first->get_class() == WATER)
        stitch_average(f, pos, star[0].first, star[0].second);
      else
        stitch_comply(f, pos, star[0].first, star[0].second, 1.0);
    }
    else if (star.size() > 1) {
      std::clog << "star size: " << star.size() << std::endl;
      //-- collect all elevations
      std::vector<float> televs;
      televs.assign(6, -999.0);
      televs[f->get_class()] = fz;
      for (auto& fadj : star) {
        // std::clog << fadj.first->get_class() << " | " 
        // << fadj.first->get_point_elevation(fadj.second) << " | " 
        // << fz 
        // << std::endl;
        if (televs[fadj.first->get_class()] < -998)
          televs[fadj.first->get_class()] = fadj.first->get_point_elevation(fadj.second);
        else {
          float tmp = (televs[fadj.first->get_class()] + fadj.first->get_point_elevation(fadj.second)) / 2;
          televs[fadj.first->get_class()] = tmp;
        }
        // if (fadj.first->get_class() == TERRAIN) {
        //   float deltaz = std::abs(fz - fadj.first->get_point_elevation(fadj.second));
        //   if (deltaz < 1.0)
        //     fadj.first->set_point_elevation(fadj.second, fz);
        //   else
        //     fadj.first->add_nc(fadj.second, fz);
        // }
      }

      process_nc(televs, 1.0);
      
      std::clog << "televs: ";
      for (auto& e : televs) {
        std::clog << "|" << e;
      }
      std::clog << std::endl;
      
      // //-- put all the features of same class at same height
      // for (auto& fadj : star) {
      //   fadj.first->set_point_elevation(fadj.second, televs[fadj.first->get_class()]);
      // }

      std::clog << televs[2] << std::endl;
    }
  }
}


void Map3d::process_nc(std::vector<float>& televs, float jumpedge) {
  for (int i = 1; i <= 5; i++) {
    for (int j = (i + 1); j <= 5; j++) {
      if ( (televs[i] >= -998) && (televs[j] >= -998) && (std::abs(televs[i] - televs[j]) < jumpedge) ) {
        televs[j] = televs[i];
      }
    }
  }
}


void Map3d::stitch_comply(TopoFeature* hard, int hardpos, TopoFeature* soft, int softpos, float jumpedge) {
  float hardz = hard->get_point_elevation(hardpos);
  float deltaz = std::abs(hardz - soft->get_point_elevation(softpos));
  std::clog << "deltaz=" << deltaz << std::endl;
  if (deltaz < jumpedge) //-- TODO 1m jump-edge?
    soft->set_point_elevation(softpos, hardz);
  else {
    std::clog << "nc added" << std::endl;
    soft->add_nc(softpos, hardz);
  }
}

void Map3d::stitch_average(TopoFeature* hard, int hardpos, TopoFeature* soft, int softpos) {
  float hardz = hard->get_point_elevation(hardpos);
  float softz = soft->get_point_elevation(softpos);
  hard->set_point_elevation(hardpos, (hardz + softz) / 2);
  soft->set_point_elevation(softpos, (hardz + softz) / 2);
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





