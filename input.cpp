
#include "input.h"


void printProgressBar( int percent ) {
  std::string bar;
  for(int i = 0; i < 50; i++){
    if( i < (percent / 2)) {
      bar.replace(i, 1, "=");
    }
    else if( i == (percent / 2)) {
      bar.replace(i, 1, ">");
    }
    else{
      bar.replace(i, 1, " ");
    }
  }
  std::cout << "\r" "[" << bar << "] ";
  std::cout.width(3);
  std::cout << percent << "%     " << std::flush;
}


std::string get_xml_header() {
  return "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
}


std::string get_citygml_namespaces() {
  return "<CityModel xmlns:veg=\"http://www.opengis.net/citygml/vegetation/2.0\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xAL=\"urn:oasis:names:tc:ciq:xsdschema:xAL:2.0\" xmlns:dem=\"http://www.opengis.net/citygml/relief/2.0\" xmlns:gml=\"http://www.opengis.net/gml\" xmlns:fme=\"http://www.safe.com/xml/xmltables\" xmlns:tran=\"http://www.opengis.net/citygml/transportation/2.0\" xmlns:bldg=\"http://www.opengis.net/citygml/building/2.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns=\"http://www.opengis.net/citygml/2.0\">";
}
