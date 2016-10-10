

    // std::vector<PairIndexed> re;
    // _rtree.query(bgi::intersects(f->get_bbox2d()), std::back_inserter(re));

    //for (auto& each : re) {
    //  TopoFeature* fadj = each.second;

    //  // if (bg::intersects(*(f->get_Polygon2()), *(fadj->get_Polygon2())))
    //  // {
    //  //   std::clog << f->get_id() << " intersects " << fadj->get_id() << std::endl;
    //  // }
    //  // if (bg::touches(*(f->get_Polygon2()), *(fadj->get_Polygon2())))
    //  // {
    //  //   std::clog << f->get_id() << " touches " << fadj->get_id() << std::endl;
    //  // }

    //  //if (fadj->get_top_level() == true && bg::touches(*(f->get_Polygon2()), *(fadj->get_Polygon2())) == true) {
    //  if (f != fadj && (bg::touches(*(f->get_Polygon2()), *(fadj->get_Polygon2())) || !bg::disjoint(*(f->get_Polygon2()), *(fadj->get_Polygon2())))) {
    //    lstouching.push_back(fadj);
    //  }
    //}


// unordered_map::insert
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <string>
//#include <fstream>
#include <sstream>
#include <iomanip>
//#include <algorithm>

typedef std::unordered_multimap<std::string,int> stringmap;

std::string genkey(double x, double y) {
    std::stringstream ss;
//    ss << std::setprecision(3) << std::fixed << "v " << bg::get<0>(v)
    
}

int main ()
{
    std::unordered_multimap<std::string, int> umm;
    
    umm.emplace("111222", 44);
    umm.emplace("111222", 45);
    umm.emplace("111", 666);
    std::cout << umm.count("111222") << std::endl;
    auto range = umm.equal_range("111222");
    for_each (
              range.first,
              range.second,
              [](stringmap::value_type& x){std::cout << " " << x.second;}
              );
    
    std::cout << std::endl << "---" << std::endl;
    if (umm.find("111222") != umm.end())
        std::cout << "found: " << umm.find("111222")->second << std::endl;
    
    std::cout << std::endl << "size: " << umm.size() << std::endl;
    return 0;
}






void TopoFeature::construct_vertical_walls() {
  int i = 0;
  for (auto& curnc : _nc) {
    if (curnc.empty() == false) {
      curnc.push_back(this->get_point_elevation(i));
      std::sort(curnc.begin(), curnc.end());
    }
    i++;
  }
  //-- add those evil vertical walls
  i = 0;
  int ni;
  for (auto& curnc : _nc) {
    this->get_next_point2_in_ring(i, ni); //-- index of next in the ring (oring or iring)
    std::vector<float> nnc = _nc[ni];
    if (nnc.size() > 0) {
      for (int j = 0; j < (nnc.size() - 1); j++) {
        _vertices_vw.push_back(Point3(this->get_point_x(i), this->get_point_y(i), this->get_point_elevation(i)));
        _vertices_vw.push_back(Point3(this->get_point_x(ni), this->get_point_y(ni), nnc[j]));
        _vertices_vw.push_back(Point3(this->get_point_x(ni), this->get_point_y(ni), nnc[j + 1]));
        Triangle t;
        t.v0 = int(_vertices_vw.size()) - 3;
        t.v1 = int(_vertices_vw.size()) - 1;
        t.v2 = int(_vertices_vw.size()) - 2;
        _triangles_vw.push_back(t);
      }
    }
    if (curnc.size() > 0) {
      for (int j = 0; j < (curnc.size() - 1); j++) {
        if (nnc.size() == 0)
          _vertices_vw.push_back(Point3(this->get_point_x(ni), this->get_point_y(ni), this->get_point_elevation(ni)));
        else
          _vertices_vw.push_back(Point3(this->get_point_x(ni), this->get_point_y(ni), nnc.front()));
        _vertices_vw.push_back(Point3(this->get_point_x(i), this->get_point_y(i), curnc[j]));
        _vertices_vw.push_back(Point3(this->get_point_x(i), this->get_point_y(i), curnc[j + 1]));
        Triangle t;
        t.v0 = int(_vertices_vw.size()) - 3;
        t.v1 = int(_vertices_vw.size()) - 2;
        t.v2 = int(_vertices_vw.size()) - 1;
        _triangles_vw.push_back(t);
      }
    }
    i++;
  }
}

//  std::vector<Polygon3d*> p3s = map3d.get_polygons3d();
//  Polygon3d* p3 = p3s[0];
//  Polygon2* p2 = p3->get_Polygon2();
//  std::cout << bg::area(*p2) << std::endl;
//  bg::reverse(*p2);
//  std::cout << bg::area(*p2) << std::endl;
//  std::cout << bg::num_interior_rings(*p2) << std::endl;
////  std::cout <<  << std::endl;
//  auto r = bg::exterior_ring(*p2);
//  std::cout << bg::wkt(r[0]) << r.size() << std::endl;
  
//  int i = 1;
//  
//  for (int i = 0; i < (r.size() - 1); i++)
//    std::cout << i << " :" << bg::get<0>(r[i]) << bg::wkt(r[i]) << std::endl;
//  auto it = r.end();
//  it--;
//  for (auto it = r.begin(); it != --r.end(); it++) {
//    std::cout << i << " :" << bg::wkt(*it) << std::endl;
//    i++;
//  }
//  for (auto& v : bg::exterior_ring(*p2) )
//  {
//    std::cout << i << " :" << bg::wkt(v) << std::endl;
//    i++;
//  }
  
//boost::geometry::unique(poly); //-- remove duplicates



//  n = nodes["output"];
//  std::cout << n.Type() << std::endl;
//  switch (n.Type()) {
//      case YAML::NodeType::Null: // ...
//          std::cout << "null" << std::endl;
//      case YAML::NodeType::Scalar: // ...
//          std::cout << "scalar" << std::endl;
//      case YAML::NodeType::Sequence: // ...
//          std::cout << "sequence" << std::endl;
//      case YAML::NodeType::Map: // ...
//          std::cout << "map" << std::endl;
//          std::cout << n.size() << std::endl;
//          for (auto it = n.begin(); it != n.end(); it++) {
//             std::cout << it->first << std::endl;
//          }
//          break;
//      case YAML::NodeType::Undefined: // ...
//          std::cout << "fuck knows" << std::endl;
//  }
//  std::cout << "\n=========\n" << std::endl;