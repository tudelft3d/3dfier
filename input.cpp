
#include "input.h"


bool read_gml_file(std::string ifile, std::vector<std::string>& layers, std::string idfield, std::vector<Polygon3d*>& lsPolys) {
  std::cout << "Reading input dataset: " << ifile << std::endl;
  OGRDataSource *dataSource = OGRSFDriverRegistrar::Open(ifile.c_str(), false);
  if (dataSource == NULL) {
    std::cerr << "Error: Could not open file." << std::endl;
    return false;
  }
  for (std::string layer : layers) {
    OGRLayer *dataLayer = dataSource->GetLayerByName(layer.c_str());
    if (dataLayer == NULL)
      continue;
    dataLayer->ResetReading();
    unsigned int numberOfPolygons = dataLayer->GetFeatureCount(true);
    std::cout << "\tLayer: " << dataLayer->GetName() << std::endl;
    std::cout << "\t(" << numberOfPolygons << " features)" << std::endl;
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
          Polygon3d* p3 = new Polygon3d(p2, SIMPLE_AVG, f->GetFieldAsString(idfield.c_str()));
          lsPolys.push_back(p3);
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
