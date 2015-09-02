//  std::vector<Polygon3d*> p3s = map3d.get_polygons3d();
//  Polygon3d* p3 = p3s[0];
//  Polygon2d* p2 = p3->get_polygon2d();
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
