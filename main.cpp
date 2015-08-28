

#include "input.h"
#include "definitions.h"
#include "Polygon3d.h"



//----------
bool readlas(std::string ifile);
bool build_rtree(std::vector<Polygon3d*>& lsPolys);
//----------



int main(int argc, const char * argv[]) {
  OGRRegisterAll();
  std::vector<std::string> layers;
  layers.push_back("Terrein");
  layers.push_back("Waterdeel");
  layers.push_back("Wegdeel");
  layers.push_back("Gebouw");
  layers.push_back("Gebouw_Vlak");
  std::string ifile = "/Users/hugo/temp/000/TOP10NL_37W_00.gml";

  std::vector<Polygon3d*> lsPolys;
  read_gml_file(ifile, layers, "identificatie", lsPolys);
  std::cout << "# of polygons: " << lsPolys.size() << std::endl;
  build_rtree(lsPolys);
//  readlas("/Users/hugo/data/ahn2/g37en2.laz");
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



bool build_rtree(std::vector<Polygon3d*>& lsPolys) {
  // TODO : why 16 here? I'm clueless about that param, that's the default
  // http://www.boost.org/doc/libs/1_58_0/libs/geometry/doc/html/geometry/spatial_indexes/creation_and_modification.html
  std::cout << "Constructing the R-tree...";
  bgi::rtree< Value, bgi::rstar<16> > rtree;
  for (auto p: lsPolys) 
    rtree.insert(std::make_pair(p->get_bbox2d(), p->get_id()));
  std::cout << " done." << std::endl;
  std::vector<Value> re;
  Point2d queryp(74659.1,447682.4);
//  Point queryp(3965657,3238869);
  Point2d minp(bg::get<0>(queryp) - 1.0, bg::get<1>(queryp) - 1.0);
  Point2d maxp(bg::get<0>(queryp) + 1.0, bg::get<1>(queryp) + 1.0);
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


