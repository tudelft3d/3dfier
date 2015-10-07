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