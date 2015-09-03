#ifndef INPUT_H
#define INPUT_H


#include "definitions.h"
#include "Polygon3d.h"

void printProgressBar( int percent );
std::string get_xml_header();
std::string get_citygml_namespaces();

std::string get_polygon_lifted_gml(Polygon2d* p2, double height, bool reverse = false);
std::string get_extruded_line_gml(Point2d* a, Point2d* b, double high, double low, bool reverse = false);
std::string get_extruded_lod1_block_gml(Polygon2d* p2, double high, double low = 0.0);  


#endif
