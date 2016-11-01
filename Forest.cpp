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


#include "Forest.h"
#include "io.h"

bool Forest::_use_ground_points_only = false;

Forest::Forest (char *wkt, std::string pid, int simplification, float innerbuffer, bool ground_points_only) : TIN(wkt, pid, simplification, innerbuffer)
{
  _use_ground_points_only = ground_points_only;
}


bool Forest::lift() {
  TopoFeature::lift_each_boundary_vertices(0.5);
  return true;
}


bool Forest::add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  bool toadd = false;
  if (lastreturn && ((_use_ground_points_only && lasclass == LAS_GROUND) || (_use_ground_points_only == false && (lasclass != LAS_GROUND && lasclass != LAS_BUILDING)))) {
    assign_elevation_to_vertex(x, y, z, radius);
    if (_simplification <= 1)
      toadd = true;
    else {
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<int> dis(1, _simplification);
      if (dis(gen) == 1)
        toadd = true;
    }
    Point2 p(x, y);
    // Add the point to the lidar points if it is within the polygon and respecting the inner buffer size
    if (toadd && bg::within(p, *(_p2)) && (_innerbuffer == 0.0 || this->get_distance_to_boundaries(p) > _innerbuffer)) {
      _lidarpts.push_back(Point3(x, y, z));
    }
  }
  return toadd;
}


TopoClass Forest::get_class() {
  return FOREST;
}


bool Forest::is_hard() {
  return false;
}


std::string Forest::get_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<veg:PlantCover gml:id=\"";
  ss << this->get_id();
  ss << "\">" << std::endl;
  ss << "<veg:lod1MultiSurface>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << std::setprecision(3) << std::fixed;
  for (auto& t : _triangles)
    ss << get_triangle_as_gml_surfacemember(t);
  for (auto& t : _triangles_vw)
    ss << get_triangle_as_gml_surfacemember(t, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</veg:lod1MultiSurface>" << std::endl;
  ss << "</veg:PlantCover>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}


std::string Forest::get_citygml_imgeo() {
  return get_citygml();
}


std::string Forest::get_mtl() {
  return "usemtl Forest\n";
}


bool Forest::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Forest");
}
