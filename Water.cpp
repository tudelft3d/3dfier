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


 
#include "Water.h"

float Water::_heightref = 0.1;

Water::Water (char *wkt, std::string pid, float heightref) 
: Flat(wkt, pid)
{
  _heightref = heightref;
}


bool Water::lift() {
  Flat::lift_percentile(_heightref);
  return true;
}


//bool Water::add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn) {
//  if (lastreturn == true && lasclass == LAS_WATER) {
//    Flat::add_elevation_point(x, y, z, radius, lasclass, lastreturn);
//  }
//  return true;
//}


TopoClass Water::get_class() {
  return WATER;
}


bool Water::is_hard() {
  return true;
}


std::string Water::get_mtl() {
  return "usemtl Water\n";
}


std::string Water::get_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<wtr:WaterBody gml:id=\"";
  ss << this->get_id();
  ss << "\">" << std::endl;

  ss << "<wtr:lod1MultiSurface>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;

  // for (auto& t : _triangles) {
  //   char* buf = new char[200];
  //   //std::sprintf(buf, "f %i %i %i\n", t.v0 + 1 + offset, t.v1 + 1 + offset, t.v2 + 1 + offset);
  //   std::sprintf(buf, "f %lu %lu %lu\n", map[t.v0], map[t.v1], map[t.v2]);
  //   obj += std::string(buf);
  // }

//               <gml:surfaceMember>
//                 <gml:Polygon gml:id="GUID_1CDBD3A1-F60E-49EB-9650-011F3F606343_1">
//                   <gml:exterior>
//                     <gml:LinearRing>
//                       <gml:pos>78670.202 457787.426 13.189</gml:pos>
//                       <gml:pos>78671.021 457791.768 13.189</gml:pos>
//                       <gml:pos>78671.021 457791.768 6.58</gml:pos>
//                       <gml:pos>78670.202 457787.426 6.58</gml:pos>
//                       <gml:pos>78670.202 457787.426 13.189</gml:pos>
//                     </gml:LinearRing>
//                   </gml:exterior>
//                 </gml:Polygon>
//               </gml:surfaceMember>

  ss << "</wtr:WaterBody>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}


bool Water::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Water");
}
