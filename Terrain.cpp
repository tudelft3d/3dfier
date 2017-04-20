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

#include "Terrain.h"
#include "io.h"
#include <algorithm>

Terrain::Terrain(char *wkt, std::string layername, std::unordered_map<std::string, std::string> attributes, std::string pid, int simplification, float innerbuffer)
  : TIN(wkt, layername, attributes, pid, simplification, innerbuffer) {}

TopoClass Terrain::get_class() {
  return TERRAIN;
}

bool Terrain::is_hard() {
  return false;
}

std::string Terrain::get_mtl() {
  return "usemtl Terrain";
}

bool Terrain::add_elevation_point(Point2 p, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  bool toadd = false;
  if (lastreturn && lasclass == LAS_GROUND) {
    toadd = TIN::add_elevation_point(p, z, radius, lasclass, lastreturn);
  }
  return toadd;
}

bool Terrain::lift() {
  //-- lift vertices to their median of lidar points
  TopoFeature::lift_each_boundary_vertices(0.5);
  return true;
}

std::string Terrain::get_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<luse:LandUse gml:id=\"" << this->get_id() << "\">" << std::endl;
  ss << "<luse:lod1MultiSurface>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << std::setprecision(3) << std::fixed;
  for (auto& t : _triangles)
    ss << get_triangle_as_gml_surfacemember(t);
  for (auto& t : _triangles_vw)
    ss << get_triangle_as_gml_surfacemember(t, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</luse:lod1MultiSurface>" << std::endl;
  ss << "</luse:LandUse>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}

std::string Terrain::get_citygml_imgeo() {
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<imgeo:OnbegroeidTerreindeel gml:id=\"" << this->get_id() << "\">" << std::endl;
  ss << get_imgeo_object_info(this->get_id());
  ss << "<lu:lod1MultiSurface>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << std::setprecision(3) << std::fixed;
  for (auto& t : _triangles)
    ss << get_triangle_as_gml_surfacemember(t);
  for (auto& t : _triangles_vw)
    ss << get_triangle_as_gml_surfacemember(t, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</lu:lod1MultiSurface>" << std::endl;
  std::string attribute;
  if (get_attribute("bgt-fysiekvoorkomen", attribute)) {
    ss << "<imgeo:bgt-fysiekVoorkomen codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenOnbegroeidTerrein\">" << attribute /*"erf"*/ << "</imgeo:bgt-fysiekVoorkomen>" << std::endl;
  }
  if (get_attribute("onbegroeidterreindeeloptalud", attribute, "false")) {
    ss << "<imgeo:onbegroeidTerreindeelOpTalud>" << attribute << "</imgeo:onbegroeidTerreindeelOpTalud>" << std::endl;
  }
  if (get_attribute("plus-fysiekvoorkomen", attribute)) {
    ss << "<imgeo:plus-fysiekVoorkomen codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#FysiekVoorkomenOnbegroeidTerreinPlus\">" << attribute << "</imgeo:plus-fysiekVoorkomen>" << std::endl;
  }
  ss << "</imgeo:OnbegroeidTerreindeel>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}

bool Terrain::get_shape(OGRLayer* layer) {
  return TopoFeature::get_shape_features(layer, "Terrain");
}

bool Terrain::get_shape2(OGRLayer * layer, std::string classname){
    
//    std::clog << "Terrain" << std::endl;
//    OGRFeature *outFeature;
//    outFeature = OGRFeature::CreateFeature(layer->GetLayerDefn());
//    outFeature->SetField("ELMID", 0);
//    // point geometry
//
//     for (auto& vs : _vertices) {
    
//         OGRFeature *outFeature;
//         outFeature = OGRFeature::CreateFeature(layer->GetLayerDefn());
//         std::cout << vs.get<0>() << "    " << vs.get<1>() << "     " << vs.get<2>() << std::endl;
    
//         OGRPoint pt;
//         pt.setX( vs.get<0>() );
//         pt.setY( vs.get<1>() );
//         outFeature->SetGeometry(&pt);
//         
//         if (layer->CreateFeature(outFeature) != OGRERR_NONE)
//         {
//             std::cerr << "Failed to create feature " << this->get_id() << " in shapefile." << std::endl;
//             return false;
//         }
//         
//         OGRFeature::DestroyFeature(outFeature);
//     }
    
//     OGRFeature::DestroyFeature(outFeature);
    
    //    polygon geometry
//    OGRPolygon polygon = OGRPolygon();
//    OGRLinearRing ring = OGRLinearRing();
//    
//    Point2 a;
//    for (int ai = 0; ai < (bg::exterior_ring(*(_p2))).size(); ai++) {
//        a = (bg::exterior_ring(*(_p2)))[ai];
//        //        std::cout << a.get<0>()  <<  "  " << a.get<1>() << std::endl;
//        ring.addPoint(a.get<0>(), a.get<1>());
//    }
//    ring.closeRings();
//    polygon.addRing(&ring);
    
    
    
//    std::string bgtattribute;
//    float relheight=0.00;
//    float h=0.00;
//    float hbase=0.00;
//    
////    outFeature->SetField("GMLID", this->get_id().c_str());
//    outFeature->SetField("GRPID", 0);
//    outFeature->SetField("GRPNAME", "NULL");
////    outFeature->SetField("ELMID", 0);
//    if (this->get_attribute("creationdate", bgtattribute)) {
//        outFeature->SetField("DATE", (bgtattribute).c_str());
//    }
//    else{
//        outFeature->SetField("DATE", "NULL");
//    }
//    outFeature->SetField("IDENT", classname.c_str());
//    outFeature->SetField("DESCR", "Building in 37EN/1 or 37EN/2");
//    outFeature->SetField("SHAPE", 1);
////    outFeature->SetField("X1", a.get<0>() );
////    outFeature->SetField("Y1", a.get<1>() );
//    
////    h = z_to_float(this->get_height());
////    outFeature->SetField("HEIGHT",  h); //roof height
////    outFeature->SetField("REL_H",  h);
//    
////    hbase = z_to_float(this->get_height_base());
////    outFeature->SetField("GRNDLVL", hbase); //floor height
////    outFeature->SetField("HDEF", 0);
//    //    outFeature->SetField("DESIGN_USE", "NULL");
    
    
    
    
//    return true;
    
      OGRFeatureDefn *featureDefn = layer->GetLayerDefn();
      OGRFeature *feature = OGRFeature::CreateFeature(featureDefn);
      OGRMultiPolygon multipolygon = OGRMultiPolygon();
      Point3 p;
    
      //-- add all triangles to the layer
      for (auto& t : _triangles) {
        OGRPolygon polygon = OGRPolygon();
        OGRLinearRing ring = OGRLinearRing();
    
        p = _vertices[t.v0];
        ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
        p = _vertices[t.v1];
        ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
        p = _vertices[t.v2];
        ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
    
        ring.closeRings();
        polygon.addRing(&ring);
        multipolygon.addGeometry(&polygon);
      }
    
//      //-- add all vertical wall triangles to the layer
//      for (auto& t : _triangles_vw) {
//        OGRPolygon polygon = OGRPolygon();
//        OGRLinearRing ring = OGRLinearRing();
//    
//        p = _vertices[t.v0];
//        ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
//        p = _vertices[t.v1];
//        ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
//        p = _vertices[t.v2];
//        ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
//    
//        ring.closeRings();
//        polygon.addRing(&ring);
//        multipolygon.addGeometry(&polygon);
//      }
    
    
      feature->SetGeometry(&multipolygon);
//      feature->SetField("Id", this->get_id().c_str());
//      feature->SetField("Class", "Terrain");
//      feature->SetField("FloorHeight", this->get_height_base());
//      feature->SetField("RoofHeight", this->get_height());
    
      if (layer->CreateFeature(feature) != OGRERR_NONE) {
        std::cerr << "Failed to create feature " << this->get_id() << " in shapefile." << std::endl;
        return false;
      }
      OGRFeature::DestroyFeature(feature);
        return true;

    
}

