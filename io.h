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

#ifndef INPUT_H
#define INPUT_H

#include "definitions.h"
#include "TopoFeature.h"

void printProgressBar(int percent);
void get_xml_header(std::ostream& of);
void get_citygml_namespaces(std::ostream& of);
void get_citygml_imgeo_namespaces(std::ostream& of);

void get_polygon_lifted_gml(std::ostream& of, Polygon2* p2, double height, bool reverse = false);
void get_extruded_line_gml(std::ostream& of, Point2* a, Point2* b, double high, double low, bool reverse = false);
void get_extruded_lod1_block_gml(std::ostream& of, Polygon2* p2, double high, double low = 0.0);

bool  is_string_integer(std::string s, int min = 0, int max = 1e6);
float z_to_float(int z);
std::vector<std::string> stringsplit(std::string str, char delimiter);

#endif
