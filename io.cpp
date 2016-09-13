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


std::string gen_key_bucket(Point2* p) {
  return std::to_string(int(bg::get<0>(p) * 100)) + std::to_string(int(bg::get<1>(p) * 100));
}


std::string gen_key_bucket(Point3* p) {
  return std::to_string(int(bg::get<0>(p) * 100)) + "-" + std::to_string(int(bg::get<1>(p) * 100)) + "-" + std::to_string(int(bg::get<2>(p) * 100));
}


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
  std::clog << "\r" "[" << bar << "] ";
  std::clog.width(3);
  std::clog << percent << "%     " << std::flush;
}


std::string get_xml_header() {
  return "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
}


std::string get_citygml_namespaces() {
  return "<CityModel xmlns:veg=\"http://www.opengis.net/citygml/vegetation/2.0\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xAL=\"urn:oasis:names:tc:ciq:xsdschema:xAL:2.0\" xmlns:dem=\"http://www.opengis.net/citygml/relief/2.0\" xmlns:gml=\"http://www.opengis.net/gml\" xmlns:fme=\"http://www.safe.com/xml/xmltables\" xmlns:tran=\"http://www.opengis.net/citygml/transportation/2.0\" xmlns:bldg=\"http://www.opengis.net/citygml/building/2.0\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns=\"http://www.opengis.net/citygml/2.0\">";
}

std::string get_polygon_lifted_gml(Polygon2* p2, double height, bool reverse) {
  std::stringstream ss;
  ss << "<gml:surfaceMember>";
  ss << "<gml:Polygon>";
  ss << "<gml:exterior>";
  ss << "<gml:LinearRing>";
  if (reverse)
    bg::reverse(*p2);
  // TODO : also do the interior rings for extrusion
  auto r = bg::exterior_ring(*p2);
  for (int i = 0; i < r.size(); i++)
    ss << "<gml:pos>" << bg::get<0>(r[i]) << " " << bg::get<1>(r[i]) << " " << height << "</gml:pos>";
  ss << "<gml:pos>" << bg::get<0>(r[r.size() - 1]) << " " << bg::get<1>(r[r.size() - 1]) << " " << height << "</gml:pos>";
  ss << "</gml:LinearRing>";
  ss << "</gml:exterior>";
  ss << "</gml:Polygon>";
  ss << "</gml:surfaceMember>";
  if (reverse)
    bg::reverse(*p2);
  return ss.str();
}

std::string get_extruded_line_gml(Point2* a, Point2* b, double high, double low, bool reverse) {
  std::stringstream ss;
  ss << "<gml:surfaceMember>";
  ss << "<gml:Polygon>";
  ss << "<gml:exterior>";
  ss << "<gml:LinearRing>";
  ss << "<gml:pos>" << bg::get<0>(b) << " " << bg::get<1>(b) << " " << low << "</gml:pos>";
  ss << "<gml:pos>" << bg::get<0>(a) << " " << bg::get<1>(a) << " " << low << "</gml:pos>";
  ss << "<gml:pos>" << bg::get<0>(a) << " " << bg::get<1>(a) << " " << high << "</gml:pos>";
  ss << "<gml:pos>" << bg::get<0>(b) << " " << bg::get<1>(b) << " " << high << "</gml:pos>";
  ss << "<gml:pos>" << bg::get<0>(b) << " " << bg::get<1>(b) << " " << low << "</gml:pos>";
  ss << "</gml:LinearRing>";
  ss << "</gml:exterior>";
  ss << "</gml:Polygon>";
  ss << "</gml:surfaceMember>";
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
    if ( (number < min) || (number > max) ) 
      return false;
  }
  catch(boost::bad_lexical_cast& e) {
    return false;
  }
  return true;
}


