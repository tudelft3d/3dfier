
#include "definitions.h"
#include "input.h"
#include "Polygon3d.h"
#include "Map3d.h"

bool readlas(std::string ifile);



int main(int argc, const char * argv[]) {
  Map3d map3d;
  map3d.add_possible_layer("Terrein");
  map3d.add_possible_layer("Waterdeel");
  map3d.add_possible_layer("Wegdeel");
  map3d.add_possible_layer("Gebouw");
  map3d.add_possible_layer("Gebouw_Vlak");
  map3d.read_gml_file("/Users/hugo/temp/000/TOP10NL_37W_00.gml", "identificatie");

  std::cout << "# of polygons: " << map3d.get_num_polygons() << std::endl;
  
  map3d.construct_rtree();
  
  std::cout << map3d.add_point(74659.1, 447682.4, 0) << std::endl;

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





