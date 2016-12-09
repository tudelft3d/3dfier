/*
  3dfier: takes 2D GIS datasets and "3dfies" to create 3D city models.

  Copyright (C) 2015-2016  3D geoinformation research group, TU Delft

  This file is part of 3dfier.

  3dfier is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  3dfier is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with 3difer.  If not, see <http://www.gnu.org/licenses/>.

  For any information or further details about the use of 3dfier, contact
  Hugo Ledoux
  <h.ledoux@tudelft.nl>
  Faculty of Architecture & the Built Environment
  Delft University of Technology
  Julianalaan 134, Delft 2628BL, the Netherlands
*/

#include "io.h"

void printProgressBar(int percent) {
  std::string bar;
  for (int i = 0; i < 50; i++) {
    if (i < (percent / 2)) {
      bar.replace(i, 1, "=");
    }
    else if (i == (percent / 2)) {
      bar.replace(i, 1, ">");
    }
    else {
      bar.replace(i, 1, " ");
    }
  }
  std::clog << "\r" "[" << bar << "] ";
  std::clog.width(3);
  std::clog << percent << "%     " << std::flush;
}

std::string get_xml_header() {
  return "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
}

std::string get_citygml_namespaces() {
  std::stringstream ss;
  ss << "<CityModel xmlns=\"http://www.opengis.net/citygml/2.0\"" << std::endl;
  ss << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << std::endl;
  ss << "xmlns:xAL=\"urn:oasis:names:tc:ciq:xsdschema:xAL:2.0\"" << std::endl;
  ss << "xmlns:xlink=\"http://www.w3.org/1999/xlink\"" << std::endl;
  ss << "xmlns:gml=\"http://www.opengis.net/gml\"" << std::endl;
  ss << "xmlns:bldg=\"http://www.opengis.net/citygml/building/2.0\"" << std::endl;
  ss << "xmlns:wtr=\"http://www.opengis.net/citygml/waterbody/2.0\"" << std::endl;
  ss << "xmlns:veg=\"http://www.opengis.net/citygml/vegetation/2.0\"" << std::endl;
  ss << "xmlns:dem=\"http://www.opengis.net/citygml/relief/2.0\"" << std::endl;
  ss << "xmlns:tran=\"http://www.opengis.net/citygml/transportation/2.0\"" << std::endl;
  ss << "xmlns:luse=\"http://www.opengis.net/citygml/landuse/2.0\"" << std::endl;
  ss << "xmlns:gen=\"http://www.opengis.net/citygml/generics/2.0\"" << std::endl;
  ss << "xmlns:brg=\"http://www.opengis.net/citygml/bridge/2.0\"" << std::endl;
  ss << "xmlns:app=\"http://www.opengis.net/citygml/appearance/2.0\"" << std::endl;
  ss << "xmlns:tun=\"http://www.opengis.net/citygml/tunnel/2.0\"" << std::endl;
  ss << "xmlns:cif=\"http://www.opengis.net/citygml/cityfurniture/2.0\"" << std::endl;
  ss << "xsi:schemaLocation=\"http://www.opengis.net/citygml/2.0 ./CityGML_2.0/CityGML.xsd\">";
  return ss.str();
}

std::string get_citygml_imgeo_namespaces() {
  std::stringstream ss;
  ss << "<CityModel xmlns=\"http://www.opengis.net/citygml/2.0\"" << std::endl;
  ss << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << std::endl;
  ss << "xmlns:xAL=\"urn:oasis:names:tc:ciq:xsdschema:xAL:2.0\"" << std::endl;
  ss << "xmlns:xlink=\"http://www.w3.org/1999/xlink\"" << std::endl;
  ss << "xmlns:gml=\"http://www.opengis.net/gml\"" << std::endl;
  ss << "xmlns:bui=\"http://www.opengis.net/citygml/building/2.0\"" << std::endl;
  ss << "xmlns:wtr=\"http://www.opengis.net/citygml/waterbody/2.0\"" << std::endl;
  ss << "xmlns:veg=\"http://www.opengis.net/citygml/vegetation/2.0\"" << std::endl;
  ss << "xmlns:dem=\"http://www.opengis.net/citygml/relief/2.0\"" << std::endl;
  ss << "xmlns:tra=\"http://www.opengis.net/citygml/transportation/2.0\"" << std::endl;
  ss << "xmlns:lu=\"http://www.opengis.net/citygml/landuse/2.0\"" << std::endl;
  ss << "xmlns:gen=\"http://www.opengis.net/citygml/generics/2.0\"" << std::endl;
  ss << "xmlns:bri=\"http://www.opengis.net/citygml/bridge/2.0\"" << std::endl;
  ss << "xmlns:app=\"http://www.opengis.net/citygml/appearance/2.0\"" << std::endl;
  ss << "xmlns:tun=\"http://www.opengis.net/citygml/tunnel/2.0\"" << std::endl;
  ss << "xmlns:cif=\"http://www.opengis.net/citygml/cityfurniture/2.0\"" << std::endl;
  ss << "xmlns:imgeo=\"http://www.geostandaarden.nl/imgeo/2.1\"" << std::endl;
  ss << "xsi:schemaLocation=\"http://www.opengis.net/citygml/2.0 http://schemas.opengis.net/citygml/2.0/cityGMLBase.xsd http://www.geostandaarden.nl/imgeo/2.1 http://schemas.geonovum.nl/imgeo/2.1/imgeo-2.1.1.xsd\">";
  return ss.str();
}

std::string get_polygon_lifted_gml(Polygon2* p2, double height, bool reverse) {
  std::stringstream ss;
  ss << std::setprecision(3) << std::fixed;
  if (reverse)
    bg::reverse(*p2);
  ss << "<gml:surfaceMember>" << std::endl;
  ss << "<gml:Polygon>" << std::endl;
  //-- oring  
  auto r = bg::exterior_ring(*p2);
  ss << "<gml:exterior>" << std::endl;
  ss << "<gml:LinearRing>" << std::endl;
  for (int i = 0; i < r.size(); i++)
    ss << "<gml:pos>" << bg::get<0>(r[i]) << " " << bg::get<1>(r[i]) << " " << height << "</gml:pos>" << std::endl;
  ss << "<gml:pos>" << bg::get<0>(r[0]) << " " << bg::get<1>(r[0]) << " " << height << "</gml:pos>" << std::endl;
  ss << "</gml:LinearRing>" << std::endl;
  ss << "</gml:exterior>" << std::endl;
  //-- irings
  auto irings = bg::interior_rings(*p2);
  for (Ring2& r : irings) {
    ss << "<gml:interior>" << std::endl;
    ss << "<gml:LinearRing>" << std::endl;
    for (int i = 0; i < r.size(); i++)
      ss << "<gml:pos>" << bg::get<0>(r[i]) << " " << bg::get<1>(r[i]) << " " << height << "</gml:pos>" << std::endl;
    ss << "<gml:pos>" << bg::get<0>(r[0]) << " " << bg::get<1>(r[0]) << " " << height << "</gml:pos>" << std::endl;
    ss << "</gml:LinearRing>" << std::endl;
    ss << "</gml:interior>" << std::endl;
  }
  ss << "</gml:Polygon>" << std::endl;
  ss << "</gml:surfaceMember>" << std::endl;
  if (reverse)
    bg::reverse(*p2);
  return ss.str();
}

std::string get_extruded_line_gml(Point2* a, Point2* b, double high, double low, bool reverse) {
  std::stringstream ss;
  ss << std::setprecision(3) << std::fixed;
  ss << "<gml:surfaceMember>" << std::endl;
  ss << "<gml:Polygon>" << std::endl;
  ss << "<gml:exterior>" << std::endl;
  ss << "<gml:LinearRing>" << std::endl;
  ss << "<gml:pos>" << bg::get<0>(b) << " " << bg::get<1>(b) << " " << low << "</gml:pos>" << std::endl;
  ss << "<gml:pos>" << bg::get<0>(a) << " " << bg::get<1>(a) << " " << low << "</gml:pos>" << std::endl;
  ss << "<gml:pos>" << bg::get<0>(a) << " " << bg::get<1>(a) << " " << high << "</gml:pos>" << std::endl;
  ss << "<gml:pos>" << bg::get<0>(b) << " " << bg::get<1>(b) << " " << high << "</gml:pos>" << std::endl;
  ss << "<gml:pos>" << bg::get<0>(b) << " " << bg::get<1>(b) << " " << low << "</gml:pos>" << std::endl;
  ss << "</gml:LinearRing>" << std::endl;
  ss << "</gml:exterior>" << std::endl;
  ss << "</gml:Polygon>" << std::endl;
  ss << "</gml:surfaceMember>" << std::endl;
  return ss.str();
}

std::string get_extruded_lod1_block_gml(Polygon2* p2, double high, double low) {
  std::stringstream ss;
  //-- get floor
  ss << get_polygon_lifted_gml(p2, low, false);
  //-- get roof
  ss << get_polygon_lifted_gml(p2, high, true);
  //-- get the walls
  auto r = bg::exterior_ring(*p2);
  for (int i = 0; i < (r.size() - 1); i++)
    ss << get_extruded_line_gml(&r[i], &r[i + 1], high, low, false);
  return ss.str();
}

bool is_string_integer(std::string s, int min, int max) {
  try {
    int number = boost::lexical_cast<int>(s);
    if ((number < min) || (number > max))
      return false;
  }
  catch (boost::bad_lexical_cast& e) {
    return false;
  }
  return true;
}

float z_to_float(int z) {
  return float(z) / 100;
}

std::vector<std::string> stringsplit(std::string str, char delimiter) {
  std::vector<std::string> internal;
  std::stringstream ss(str); // Turn the string into a stream.
  std::string tok;

  while (getline(ss, tok, delimiter)) {
    std::remove_if(tok.begin(), tok.end(), isspace);
    internal.push_back(tok);
  }

  return internal;
}
