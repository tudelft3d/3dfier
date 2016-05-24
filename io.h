/*
 Copyright (c) 2015 Hugo Ledoux
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#ifndef INPUT_H
#define INPUT_H


#include "definitions.h"
#include "TopoFeature.h"

void printProgressBar( int percent );
std::string get_xml_header();
std::string get_citygml_namespaces();

std::string get_polygon_lifted_gml(Polygon2* p2, double height, bool reverse = false);
std::string get_extruded_line_gml(Point2* a, Point2* b, double high, double low, bool reverse = false);
std::string get_extruded_lod1_block_gml(Polygon2* p2, double high, double low = 0.0);  

bool is_string_integer(std::string s, int min = 0, int max = 1e6);

std::string gen_key_bucket(Point2* p);

#endif
