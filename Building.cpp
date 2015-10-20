//
//  Building.cpp
//
//  Created by Hugo Ledoux on 19/10/15.
//  Copyright © 2015 Hugo Ledoux. All rights reserved.
//

#include "Building.h"
#include "io.h"


Building::Building(Polygon2* p, std::string pid, std::string heightref_top, std::string heightref_base) 
: Block(p, pid, heightref_top, heightref_base)
{}


std::string Building::get_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>";
  ss << "<bldg:Building>";
  ss << "<bldg:measuredHeight uom=\"#m\">";
  ss << this->get_height_top();
  ss << "</bldg:measuredHeight>";
  ss << "<bldg:lod1Solid>";
  ss << "<gml:Solid>";
  ss << "<gml:exterior>";
  ss << "<gml:CompositeSurface>";
  //-- get floor
  ss << get_polygon_lifted_gml(this->_p2, this->get_height_base(), false);
  //-- get roof
  ss << get_polygon_lifted_gml(this->_p2, this->get_height_top(), true);
  //-- get the walls
  auto r = bg::exterior_ring(*(this->_p2));
  for (int i = 0; i < (r.size() - 1); i++) 
    ss << get_extruded_line_gml(&r[i], &r[i + 1], this->get_height_top(), 0, false);
  ss << "</gml:CompositeSurface>";
  ss << "</gml:exterior>";
  ss << "</gml:Solid>";
  ss << "</bldg:lod1Solid>";
  ss << "</bldg:Building>";
  ss << "</cityObjectMember>";
  return ss.str(); 
}

std::string Building::get_csv() {
  std::stringstream ss;
  ss << this->get_id() << ";" << std::setprecision(2) << std::fixed << this->get_height_top() << ";" << this->get_height_base() << std::endl;
  return ss.str(); 
}

