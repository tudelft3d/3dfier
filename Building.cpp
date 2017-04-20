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

#include "Building.h"
#include "io.h"

float Building::_heightref_top = 0.9;
float Building::_heightref_base = 0.1;

Building::Building(char *wkt, std::string layername, std::unordered_map<std::string, std::string> attributes, std::string pid, float heightref_top, float heightref_base)
  : Flat(wkt, layername, attributes, pid)
{
  _heightref_top = heightref_top;
  _heightref_base = heightref_base;
}

bool Building::lift() {
  //-- for the ground
  if (_zvaluesground.empty() == false) {
    //-- Only use ground points for base height calculation
    std::nth_element(_zvaluesground.begin(), _zvaluesground.begin() + (_zvaluesground.size() * _heightref_base), _zvaluesground.end());
    _height_base = _zvaluesground[_zvaluesground.size() * _heightref_base];
  }
  else if (_zvaluesinside.empty() == false) {
    std::nth_element(_zvaluesinside.begin(), _zvaluesinside.begin() + (_zvaluesinside.size() * _heightref_base), _zvaluesinside.end());
    _height_base = _zvaluesinside[_zvaluesinside.size() * _heightref_base];
  }
  else {
    _height_base = 0;
  }

  //-- for the roof
  Flat::lift_percentile(_heightref_top);

  return true;
}

bool Building::add_elevation_point(Point2 p, double z, float radius, LAS14Class lasclass, bool lastreturn) {
  if (lastreturn) {
    if (bg::distance(p, *(_p2)) <= radius) {
      int zcm = int(z * 100);
      //-- 1. Save the ground points seperate for base height
      if (lasclass == LAS_GROUND || lasclass == LAS_WATER) {
        _zvaluesground.push_back(zcm);
      }
      //-- 2. assign to polygon since within
      _zvaluesinside.push_back(zcm);
    }
  }
  return true;
}

int Building::get_height_base() {
  return _height_base;
}

TopoClass Building::get_class() {
  return BUILDING;
}

bool Building::is_hard() {
  return true;
}

std::string Building::get_csv() {
  std::stringstream ss;
  ss << this->get_id() << ";" << std::setprecision(2) << std::fixed << this->get_height() << ";" << this->get_height_base() << std::endl;
  return ss.str();
}

std::string Building::get_mtl() {
  return "usemtl Building";
}

std::string Building::get_obj(std::unordered_map< std::string, unsigned long > &dPts, int lod, std::string mtl) {
  std::stringstream ss;
  if (lod == 1) {
    ss << TopoFeature::get_obj(dPts, mtl);
  }
  else if (lod == 0) {
    ss << mtl << std::endl;
    for (auto& t : _triangles) {
      unsigned long a, b, c;
      int z = this->get_height_base();
      auto it = dPts.find(gen_key_bucket(&_vertices[t.v0], z));
      if (it == dPts.end()) {
        a = dPts.size() + 1;
        dPts[gen_key_bucket(&_vertices[t.v0], z)] = a;
      }
      else {
        a = it->second;
      }
      it = dPts.find(gen_key_bucket(&_vertices[t.v1], z));
      if (it == dPts.end()) {
        b = dPts.size() + 1;
        dPts[gen_key_bucket(&_vertices[t.v1], z)] = b;
      }
      else {
        b = it->second;
      }
      it = dPts.find(gen_key_bucket(&_vertices[t.v2], z));
      if (it == dPts.end()) {
        c = dPts.size() + 1;
        dPts[gen_key_bucket(&_vertices[t.v2], z)] = c;
      }
      else {
        c = it->second;
      }
      if ((a != b) && (a != c) && (b != c))
        ss << "f " << a << " " << b << " " << c << std::endl;
      // else
      //   std::clog << "COLLAPSED TRIANGLE REMOVED" << std::endl;
    }
  }
  return ss.str();
}

std::string Building::get_citygml() {
  float h = z_to_float(this->get_height());
  float hbase = z_to_float(this->get_height_base());
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<bldg:Building gml:id=\"";
  ss << this->get_id();
  ss << "\">" << std::endl;
  ss << "<bldg:measuredHeight uom=\"#m\">";
  ss << h;
  ss << "</bldg:measuredHeight>" << std::endl;
  //-- LOD0 footprint
  ss << "<bldg:lod0FootPrint>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << get_polygon_lifted_gml(this->_p2, hbase, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</bldg:lod0FootPrint>" << std::endl;
  //-- LOD0 roofedge
  ss << "<bldg:lod0RoofEdge>" << std::endl;
  ss << "<gml:MultiSurface>" << std::endl;
  ss << get_polygon_lifted_gml(this->_p2, h, true);
  ss << "</gml:MultiSurface>" << std::endl;
  ss << "</bldg:lod0RoofEdge>" << std::endl;
  //-- LOD1 Solid
  ss << "<bldg:lod1Solid>" << std::endl;
  ss << "<gml:Solid>" << std::endl;
  ss << "<gml:exterior>" << std::endl;
  ss << "<gml:CompositeSurface>" << std::endl;
  //-- get floor
  ss << get_polygon_lifted_gml(this->_p2, hbase, false);
  //-- get roof
  ss << get_polygon_lifted_gml(this->_p2, h, true);
  //-- get the walls
  auto r = bg::exterior_ring(*(this->_p2));
  int i;
  for (i = 0; i < (r.size() - 1); i++)
    ss << get_extruded_line_gml(&r[i], &r[i + 1], h, hbase, false);
  ss << get_extruded_line_gml(&r[i], &r[0], h, hbase, false);
  //-- irings
  auto irings = bg::interior_rings(*(this->_p2));
  for (Ring2& r : irings) {
    for (i = 0; i < (r.size() - 1); i++)
      ss << get_extruded_line_gml(&r[i], &r[i + 1], h, hbase, false);
    ss << get_extruded_line_gml(&r[i], &r[0], h, hbase, false);
  }
  ss << "</gml:CompositeSurface>" << std::endl;
  ss << "</gml:exterior>" << std::endl;
  ss << "</gml:Solid>" << std::endl;
  ss << "</bldg:lod1Solid>" << std::endl;
  ss << "</bldg:Building>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}

std::string Building::get_citygml_imgeo() {
  float h = z_to_float(this->get_height());
  float hbase = z_to_float(this->get_height_base());
  std::stringstream ss;
  ss << "<cityObjectMember>" << std::endl;
  ss << "<bui:Building gml:id=\"" << this->get_id() << "\">" << std::endl;
  //-- store building information
  ss << get_imgeo_object_info(this->get_id());
  ss << "<bui:consistsOfBuildingPart>" << std::endl;
  ss << "<bui:BuildingPart>" << std::endl;
  //-- LOD1 Solid
  ss << "<bui:lod1Solid>" << std::endl;
  ss << "<gml:Solid>" << std::endl;
  ss << "<gml:exterior>" << std::endl;
  ss << "<gml:CompositeSurface>" << std::endl;
  //-- get floor
  ss << get_polygon_lifted_gml(this->_p2, hbase, false);
  //-- get roof
  ss << get_polygon_lifted_gml(this->_p2, h, true);
  //-- get the walls
  auto r = bg::exterior_ring(*(this->_p2));
  int i;
  for (i = 0; i < (r.size() - 1); i++)
    ss << get_extruded_line_gml(&r[i], &r[i + 1], h, hbase, false);
  ss << get_extruded_line_gml(&r[i], &r[0], h, hbase, false);
  //-- irings
  auto irings = bg::interior_rings(*(this->_p2));
  for (Ring2& r : irings) {
    for (i = 0; i < (r.size() - 1); i++)
      ss << get_extruded_line_gml(&r[i], &r[i + 1], h, hbase, false);
    ss << get_extruded_line_gml(&r[i], &r[0], h, hbase, false);
  }
  ss << "</gml:CompositeSurface>" << std::endl;
  ss << "</gml:exterior>" << std::endl;
  ss << "</gml:Solid>" << std::endl;
  ss << "</bui:lod1Solid>" << std::endl;
  std::string attribute;
  if (get_attribute("identificatiebagpnd", attribute)) {
    ss << "<imgeo:identificatieBAGPND>" << attribute << "</imgeo:identificatieBAGPND>" << std::endl;
  }
  ss << get_citygml_imgeo_number();
  ss << "</bui:BuildingPart>" << std::endl;
  ss << "</bui:consistsOfBuildingPart>" << std::endl;
  ss << "</bui:Building>" << std::endl;
  ss << "</cityObjectMember>" << std::endl;
  return ss.str();
}

std::string Building::get_citygml_imgeo_number() {
  std::stringstream ss;
  std::string attribute;
  bool btekst, bplaatsingspunt, bhoek, blaagnr, bhoognr;
  std::string tekst, plaatsingspunt, hoek, laagnr, hoognr;
  btekst = get_attribute("tekst", tekst);
  bplaatsingspunt = get_attribute("plaatsingspunt", plaatsingspunt);
  bhoek = get_attribute("hoek", hoek);
  blaagnr = get_attribute("identificatiebagvbolaagstehuisnummer", laagnr);
  bhoognr = get_attribute("identificatiebagvbohoogstehuisnummer", hoognr);

  if (btekst) {
    // Split the lists into vector of strings
    std::vector<std::string> tekst_split, plaatsingspunt_split, hoek_split, laagnr_split, hoognr_split;
    tekst_split = stringsplit(tekst.substr(3, tekst.size() - 4), ',');
    if (bplaatsingspunt) {
      plaatsingspunt_split = stringsplit(plaatsingspunt.substr(3, plaatsingspunt.size() - 4), ',');
    }
    if (bhoek) {
      hoek_split = stringsplit(hoek.substr(3, hoek.size() - 4), ',');
    }
    if (blaagnr) {
      laagnr_split = stringsplit(laagnr.substr(3, laagnr.size() - 4), ',');
    }
    if (bhoognr) {
      hoognr_split = stringsplit(hoognr.substr(3, hoognr.size() - 4), ',');
    }

    //Get the amount of text in the StringList and write all List values separate
    int count = boost::lexical_cast<int>(tekst[1]);
    for (int i = 0; i < count; i++) {
      if (i < tekst_split.size() && i < plaatsingspunt_split.size() && i < hoek_split.size()) {
        ss << "<imgeo:nummeraanduidingreeks>" << std::endl;
        ss << "<imgeo:Nummeraanduidingreeks>" << std::endl;
        ss << "<imgeo:nummeraanduidingreeks>" << std::endl;
        ss << "<imgeo:Label>" << std::endl;
        ss << "<imgeo:tekst>" << tekst_split.at(i) << "</imgeo:tekst>" << std::endl;
        ss << "<imgeo:positie>" << std::endl;
        ss << "<imgeo:Labelpositie>" << std::endl;
        ss << "<imgeo:plaatsingspunt><gml:Point srsDimension=\"2\"><gml:pos>" << plaatsingspunt_split.at(i) << "</gml:pos></gml:Point></imgeo:plaatsingspunt>" << std::endl;
        ss << "<imgeo:hoek>" << hoek_split.at(i) << "</imgeo:hoek>" << std::endl;
        ss << "</imgeo:Labelpositie>" << std::endl;
        ss << "</imgeo:positie>" << std::endl;
        ss << "</imgeo:Label>" << std::endl;
        ss << "</imgeo:nummeraanduidingreeks>" << std::endl;
        if (i < laagnr_split.size()) {
          ss << "<imgeo:identificatieBAGVBOLaagsteHuisnummer>" << laagnr_split.at(i) << "</imgeo:identificatieBAGVBOLaagsteHuisnummer>" << std::endl;
        }
        if (i < hoognr_split.size()) {
          ss << "<imgeo:identificatieBAGVBOHoogsteHuisnummer>" << hoognr_split.at(i) << "</imgeo:identificatieBAGVBOHoogsteHuisnummer>" << std::endl;
        }
        ss << "</imgeo:Nummeraanduidingreeks>" << std::endl;
        ss << "</imgeo:nummeraanduidingreeks>" << std::endl;
      }
    }
  }
  return ss.str();
}


bool Building::get_shape(OGRLayer* layer) {
    return TopoFeature::get_shape_features(layer, "Building");
}

bool Building::get_shape2(OGRLayer * layer, std::string classname){
    std::clog << "Building" << std::endl;
    OGRFeature *outFeature = OGRFeature::CreateFeature(layer->GetLayerDefn());
    
    //    polygon geometry
    OGRPolygon polygon = OGRPolygon();
    OGRLinearRing ring = OGRLinearRing();
    
    Point2 a;
    for (int ai = 0; ai < (bg::exterior_ring(*(_p2))).size(); ai++) {
        a = (bg::exterior_ring(*(_p2)))[ai];
//        std::cout << a.get<0>()  <<  "  " << a.get<1>() << std::endl;
        ring.addPoint(a.get<0>(), a.get<1>());
    }
    ring.closeRings();
    polygon.addRing(&ring);
    
    outFeature->SetGeometry(&polygon);
    
    std::string bgtattribute;
    float relheight=0.00;
    float h=0.00;
    float hbase=0.00;
   
    outFeature->SetField("GMLID", this->get_id().c_str());
    outFeature->SetField("GRPID", 0);
    outFeature->SetField("GRPNAME", "NULL");
//    outFeature->SetField("ELMID", 0);
    if (this->get_attribute("creationdate", bgtattribute)) {
        outFeature->SetField("DATE", (bgtattribute).c_str());
    }
    else{
        outFeature->SetField("DATE", "NULL");
    }
    outFeature->SetField("IDENT", classname.c_str());
    outFeature->SetField("DESCR", "Building in 37EN/1 or 37EN/2");
    outFeature->SetField("SHAPE", 3);
    outFeature->SetField("X1", a.get<0>() );
    outFeature->SetField("Y1", a.get<1>() );
    
    h = z_to_float(this->get_height());
    outFeature->SetField("HEIGHT",  h); //roof height
    outFeature->SetField("REL_H",  h);

    hbase = z_to_float(this->get_height_base());
    outFeature->SetField("GRNDLVL", hbase); //floor height
    outFeature->SetField("HDEF", 0);
//    outFeature->SetField("DESIGN_USE", "NULL");
    
    if (layer->CreateFeature(outFeature) != OGRERR_NONE)
    {
        std::cerr << "Failed to create feature " << this->get_id() << " in shapefile." << std::endl;
        return false;
    }

    OGRFeature::DestroyFeature(outFeature);
    return true;
}

//bool Building::get_shape(OGRLayer* layer) {
//  OGRFeatureDefn *featureDefn = layer->GetLayerDefn();
//  OGRFeature *feature = OGRFeature::CreateFeature(featureDefn);
//  OGRMultiPolygon multipolygon = OGRMultiPolygon();
//  Point3 p;
//
//  //-- add all triangles to the layer
//  for (auto& t : _triangles) {
//    OGRPolygon polygon = OGRPolygon();
//    OGRLinearRing ring = OGRLinearRing();
//
//    p = _vertices[t.v0];
//    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
//    p = _vertices[t.v1];
//    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
//    p = _vertices[t.v2];
//    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
//
//    ring.closeRings();
//    polygon.addRing(&ring);
//    multipolygon.addGeometry(&polygon);
//  }
//
//  //-- add all vertical wall triangles to the layer
//  for (auto& t : _triangles_vw) {
//    OGRPolygon polygon = OGRPolygon();
//    OGRLinearRing ring = OGRLinearRing();
//
//    p = _vertices[t.v0];
//    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
//    p = _vertices[t.v1];
//    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
//    p = _vertices[t.v2];
//    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
//
//    ring.closeRings();
//    polygon.addRing(&ring);
//    multipolygon.addGeometry(&polygon);
//  }
//   
//
//  feature->SetGeometry(&multipolygon);
//  feature->SetField("Id", this->get_id().c_str());
//  feature->SetField("Class", "Building");
//  feature->SetField("FloorHeight", this->get_height_base());
//  feature->SetField("RoofHeight", this->get_height());
//
//  if (layer->CreateFeature(feature) != OGRERR_NONE) {
//    std::cerr << "Failed to create feature " << this->get_id() << " in shapefile." << std::endl;
//    return false;
//  }
//  OGRFeature::DestroyFeature(feature);
//    return true;
//
//}
