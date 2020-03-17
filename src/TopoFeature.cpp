/*
  3dfier: takes 2D GIS datasets and "3dfies" to create 3D city models.

  Copyright (C) 2015-2020 3D geoinformation research group, TU Delft

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

#include "TopoFeature.h"

TopoFeature::TopoFeature(char *wkt, std::string layername, AttributeMap attributes, std::string pid) {
  _id = pid;
  _toplevel = true;
  _bVerticalWalls = false;
  _p2 = new Polygon2();
  bg::read_wkt(wkt, *_p2);
  bg::unique(*_p2); //-- remove duplicate vertices
  bg::correct(*_p2); //-- correct the orientation of the polygons!

  _adjFeatures = new std::vector<TopoFeature*>;
  _p2z.resize(bg::num_interior_rings(*_p2) + 1);
  _p2z[0].resize(bg::num_points(_p2->outer()));
  _lidarelevs.resize(bg::num_interior_rings(*_p2) + 1);
  _lidarelevs[0].resize(bg::num_points(_p2->outer()));
  for (int i = 0; i < bg::num_interior_rings(*_p2); i++) {
    _p2z[i + 1].resize(bg::num_points(_p2->inners()[i]));
    _lidarelevs[i + 1].resize(bg::num_points(_p2->inners()[i]));
  }
  _attributes = attributes;
  _layername = layername;
}

TopoFeature::~TopoFeature() {
  // TODO: clear memory properly
}

Box2 TopoFeature::get_bbox2d() {
  return bg::return_envelope<Box2>(*_p2);
}

std::string TopoFeature::get_id() {
  return _id;
}

std::string  TopoFeature::get_layername() {
  return _layername;
}

bool TopoFeature::buildCDT() {
  return getCDT(_p2, _p2z, _vertices, _triangles);
}

bool TopoFeature::get_top_level() {
  return _toplevel;
}

void TopoFeature::set_top_level(bool toplevel) {
  _toplevel = toplevel;
}

Polygon2* TopoFeature::get_Polygon2() {
  return _p2;
}

void TopoFeature::get_cityjson_geom(nlohmann::json& g, std::unordered_map<std::string,unsigned long> &dPts, std::string primitive) {
  g["type"] = primitive;
  g["lod"] = 1;
  g["boundaries"];
  std::vector<std::vector<std::vector<unsigned long>>> shelli;
  for (auto& t : _triangles) {
    unsigned long a, b, c;
    auto it = dPts.find(_vertices[t.v0].second);
    if (it == dPts.end()) {
      a = dPts.size();
      dPts[_vertices[t.v0].second] = a;
    }
    else
      a = it->second;
    it = dPts.find(_vertices[t.v1].second);
    if (it == dPts.end()) {
      b = dPts.size();
      dPts[_vertices[t.v1].second] = b;
    }
    else
      b = it->second;
    it = dPts.find(_vertices[t.v2].second);
    if (it == dPts.end()) {
      c = dPts.size();
      dPts[_vertices[t.v2].second] = c;
    }
    else
      c = it->second;
    if ((a != b) && (a != c) && (b != c))
      shelli.push_back({{a, b, c}});
  }
  for (auto& t : _triangles_vw) {
    unsigned long a, b, c;
    auto it = dPts.find(_vertices_vw[t.v0].second);
    if (it == dPts.end()) {
      a = dPts.size();
      dPts[_vertices_vw[t.v0].second] = a;
    }
    else
      a = it->second;
    it = dPts.find(_vertices_vw[t.v1].second);
    if (it == dPts.end()) {
      b = dPts.size();
      dPts[_vertices_vw[t.v1].second] = b;
    }
    else
      b = it->second;
    it = dPts.find(_vertices_vw[t.v2].second);
    if (it == dPts.end()) {
      c = dPts.size();
      dPts[_vertices_vw[t.v2].second] = c;
    }
    else
      c = it->second;
    if ((a != b) && (a != c) && (b != c)) 
      shelli.push_back({{a, b, c}});
  }
  if (primitive == "MultiSurface")
    g["boundaries"] = shelli;
  else
    g["boundaries"].push_back(shelli);
}

void TopoFeature::get_obj(std::unordered_map< std::string, unsigned long > &dPts, std::string mtl, std::string &fs) {
  fs += mtl; fs += "\n";
  for (auto& t : _triangles) {
    unsigned long a, b, c;
    auto it = dPts.find(_vertices[t.v0].second);
    if (it == dPts.end()) {
      // first get the size + 1 and then store the size in dPts due to unspecified order of execution
      a = dPts.size() + 1;
      dPts[_vertices[t.v0].second] = a;
    }
    else
      a = it->second;
    it = dPts.find(_vertices[t.v1].second);
    if (it == dPts.end()) {
      b = dPts.size() + 1;
      dPts[_vertices[t.v1].second] = b;
    }
    else
      b = it->second;
    it = dPts.find(_vertices[t.v2].second);
    if (it == dPts.end()) {
      c = dPts.size() + 1;
      dPts[_vertices[t.v2].second] = c;
    }
    else
      c = it->second;

    if ((a != b) && (a != c) && (b != c)) {
      fs += "f "; fs += std::to_string(a); fs += " "; fs += std::to_string(b); fs += " "; fs += std::to_string(c); fs += "\n";
    }
  }

  //-- vertical triangles
  if (_triangles_vw.size() > 0) {
    fs += mtl; fs += "Wall"; fs += "\n";
    for (auto& t : _triangles_vw) {
      unsigned long a, b, c;
      auto it = dPts.find(_vertices_vw[t.v0].second);
      if (it == dPts.end()) {
        a = dPts.size() + 1;
        dPts[_vertices_vw[t.v0].second] = a;
      }
      else
        a = it->second;
      it = dPts.find(_vertices_vw[t.v1].second);
      if (it == dPts.end()) {
        b = dPts.size() + 1;
        dPts[_vertices_vw[t.v1].second] = b;
      }
      else
        b = it->second;
      it = dPts.find(_vertices_vw[t.v2].second);
      if (it == dPts.end()) {
        c = dPts.size() + 1;
        dPts[_vertices_vw[t.v2].second] = c;
      }
      else
        c = it->second;

      if ((a != b) && (a != c) && (b != c)) {
        fs += "f "; fs += std::to_string(a); fs += " "; fs += std::to_string(b); fs += " "; fs += std::to_string(c); fs += "\n";
      }
    }
  }
}

AttributeMap &TopoFeature::get_attributes() {
  return _attributes;
}

void TopoFeature::get_imgeo_attributes(std::wostream& of, std::string id) {
  std::string attribute;
  if (get_attribute("creationDate", attribute)) {
    of << "<imgeo:creationDate>" << attribute << "</imgeo:creationDate>";
  }
  if (get_attribute("terminationDate", attribute)) {
    of << "<imgeo:terminationDate>" << attribute << "</imgeo:terminationDate>";
  }
  if (get_attribute("lokaalid", attribute)) {
    of << "<imgeo:identificatie>";
    of << "<imgeo:NEN3610ID>";
    of << "<imgeo:namespace>NL.IMGeo</imgeo:namespace>";
    of << "<imgeo:lokaalID>" << attribute << "</imgeo:lokaalID>";
    of << "</imgeo:NEN3610ID>";
    of << "</imgeo:identificatie>";
  }
  if (get_attribute("tijdstipregistratie", attribute)) {
    of << "<imgeo:tijdstipRegistratie>" << attribute << "</imgeo:tijdstipRegistratie>";
  }
  if (get_attribute("eindregistratie", attribute)) {
    of << "<imgeo:eindRegistratie>" << attribute << "</imgeo:eindRegistratie>";
  }
  if (get_attribute("lv-publicatiedatum", attribute)) {
    of << "<imgeo:LV-publicatiedatum>" << attribute << "</imgeo:LV-publicatiedatum>";
  }
  else if (get_attribute("lv_publicatiedatum", attribute)) {
    of << "<imgeo:LV-publicatiedatum>" << attribute << "</imgeo:LV-publicatiedatum>";
  }
  if (get_attribute("bronhouder", attribute)) {
    of << "<imgeo:bronhouder>" << attribute << "</imgeo:bronhouder>";
  }
  if (get_attribute("inonderzoek", attribute)) {
    of << "<imgeo:inOnderzoek>" << attribute << "</imgeo:inOnderzoek>";
  }
  if (get_attribute("relatievehoogteligging", attribute)) {
    of << "<imgeo:relatieveHoogteligging>" << attribute << "</imgeo:relatieveHoogteligging>";
  }
  if (get_attribute("bgt-status", attribute, "bestaand")) {
    of << "<imgeo:bgt-status codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#Status\">" << attribute << "</imgeo:bgt-status>";
  }
  else if (get_attribute("bgt_status", attribute, "bestaand")) {
    of << "<imgeo:bgt-status codeSpace=\"http://www.geostandaarden.nl/imgeo/def/2.1#Status\">" << attribute << "</imgeo:bgt-status>";
  }
  if (get_attribute("plus-status", attribute)) {
    of << "<imgeo:plus-status>" << attribute << "</imgeo:plus-status>";
  }
  else if (get_attribute("plus_status", attribute)) {
      of << "<imgeo:plus-status>" << attribute << "</imgeo:plus-status>";
    }
}

void TopoFeature::get_cityjson_attributes(nlohmann::json& f, const AttributeMap& attributes) {
  for (auto& attribute : attributes) {
    // add attributes except gml_id
    if (attribute.first.compare("gml_id") != 0) 
      f["attributes"][std::get<0>(attribute)] = attribute.second.second;
  }
}

void TopoFeature::get_citygml_attributes(std::wostream& of, const AttributeMap& attributes) {
  for (auto& attribute : attributes) {
    // add attributes except gml_id
    if (attribute.first.compare("gml_id") != 0) {
      std::string type;
      switch (attribute.second.first) {
      case OFTInteger:
        type = "int";
      case OFTReal:
        type = "double";
      case OFTDate:
        type = "date";
      default:
        type = "string";
      }
      of << "<gen:" + type + "Attribute name=\"" + std::get<0>(attribute) + "\">";
      of << "<gen:value>" + attribute.second.second + "</gen:value>";
      of << "</gen:" + type << "Attribute>";
    }
  }
}

bool TopoFeature::get_multipolygon_features(OGRLayer* layer, std::string className, bool writeAttributes, const AttributeMap& extraAttributes) {
  OGRFeatureDefn *featureDefn = layer->GetLayerDefn();
  OGRFeature *feature = OGRFeature::CreateFeature(featureDefn);
  OGRMultiPolygon multipolygon = OGRMultiPolygon();
  Point3 p;

  //-- add all triangles to the layer
  for (auto& t : _triangles) {
    OGRPolygon polygon = OGRPolygon();
    OGRLinearRing ring = OGRLinearRing();

    p = _vertices[t.v0].first;
    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
    p = _vertices[t.v1].first;
    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
    p = _vertices[t.v2].first;
    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());

    ring.closeRings();
    polygon.addRing(&ring);
    multipolygon.addGeometry(&polygon);
  }

  //-- add all vertical wall triangles to the layer
  for (auto& t : _triangles_vw) {
    OGRPolygon polygon = OGRPolygon();
    OGRLinearRing ring = OGRLinearRing();

    p = _vertices_vw[t.v0].first;
    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
    p = _vertices_vw[t.v1].first;
    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());
    p = _vertices_vw[t.v2].first;
    ring.addPoint(p.get<0>(), p.get<1>(), p.get<2>());

    ring.closeRings();
    polygon.addRing(&ring);
    multipolygon.addGeometry(&polygon);
  }

  if (feature->SetGeometry(&multipolygon) != OGRERR_NONE) {
    std::cerr << "Creating feature geometry failed.\n";
    OGRFeature::DestroyFeature(feature);
    return false;
  }
  if (!writeAttribute(feature, featureDefn, "3df_id", this->get_id())) {
    return false;
  }
  if (!writeAttribute(feature, featureDefn, "3df_class", className)) {
    return false;
  }
  if (writeAttributes) {
    for (auto attr : _attributes) {
      if (!(attr.second.first == OFTDateTime && attr.second.second == "0000/00/00 00:00:00")) {
        if (!writeAttribute(feature, featureDefn, attr.first, attr.second.second)) {
          return false;
        }
      }
    }
    for (auto attr : extraAttributes) {
      if (!writeAttribute(feature, featureDefn, attr.first, attr.second.second)) {
        return false;
      }
    }
  }
  if (layer->CreateFeature(feature) != OGRERR_NONE) {
    std::cerr << "Failed to create feature " << this->get_id() << ".\n";
    OGRFeature::DestroyFeature(feature);
    return false;
  }
  OGRFeature::DestroyFeature(feature);
  return true;
}

bool TopoFeature::writeAttribute(OGRFeature* feature, OGRFeatureDefn* featureDefn, std::string name, std::string value) {
  int fi = featureDefn->GetFieldIndex(name.c_str());
  if (fi == -1) {
    // try replace '-' with '_' for postgresql column names
    std::replace(name.begin(), name.end(), '-', '_');
    fi = featureDefn->GetFieldIndex(name.c_str());
    if (fi == -1) {
      std::cerr << "Failed to write attribute " << name << ".\n";
      return false;
    }
  }
  // perform extra character encoding for gdal.
  char* attrcpl = CPLRecode(value.c_str(), "", CPL_ENC_UTF8);
  feature->SetField(fi, attrcpl);
  CPLFree(attrcpl);
  return true;
}

void TopoFeature::fix_bowtie() {
  //-- gather all rings
  std::vector<Ring2> therings;
  therings.push_back(_p2->outer());
  for (Ring2& iring : _p2->inners())
    therings.push_back(iring);

  //-- process each vertex of the polygon separately
  std::vector<int> anc, bnc;
  Point2 a, b;
  TopoFeature* fadj;
  int ringi = -1;
  for (Ring2& ring : therings) {
    ringi++;
    for (int ai = 0; ai < ring.size(); ai++) {
      //-- Point a
      a = ring[ai];
      //-- find Point b
      int bi;
      if (ai == (ring.size() - 1)) {
        b = ring.front();
        bi = 0;
      }
      else {
        b = ring[ai + 1];
        bi = ai + 1;
      }
      //-- find the adjacent polygon to segment ab (fadj)
      fadj = nullptr;
      int adj_a_ringi = 0;
      int adj_a_pi = 0;
      int adj_b_ringi = 0;
      int adj_b_pi = 0;
      for (auto& adj : *(_adjFeatures)) {
        if (adj->has_segment(b, a, adj_b_ringi, adj_b_pi, adj_a_ringi, adj_a_pi) == true) {
          // if (adj->has_segment(b, a) == true) {
          fadj = adj;
          break;
        }
      }
      if (fadj == nullptr)
        continue;
      //-- check height differences: f > fadj for *both* Points a and b
      int az = this->get_vertex_elevation(ringi, ai);
      int bz = this->get_vertex_elevation(ringi, bi);
      int fadj_az = fadj->get_vertex_elevation(adj_a_ringi, adj_a_pi);
      int fadj_bz = fadj->get_vertex_elevation(adj_b_ringi, adj_b_pi);

      //-- Fix bow-ties
      if (((az > fadj_az) && (bz < fadj_bz)) || ((az < fadj_az) && (bz > fadj_bz))) {
        if (this->is_hard() && fadj->is_hard() == false) {
          //- this is hard, snap the smallest height of the soft feature to this
          if (abs(az - fadj_az) < abs(bz - fadj_bz)) {
            fadj->set_vertex_elevation(adj_a_ringi, adj_a_pi, az);
          }
          else {
            fadj->set_vertex_elevation(adj_b_ringi, adj_b_pi, bz);
          }
        }
        else if (this->is_hard() == false && fadj->is_hard()) {
          //- this is soft, snap the smallest height to the hard feature
          if (abs(az - fadj_az) < abs(bz - fadj_bz)) {
            this->set_vertex_elevation(ringi, ai, fadj_az);
          }
          else {
            this->set_vertex_elevation(ringi, bi, fadj_bz);
          }
        }
        else {
          if (abs(az - fadj_az) < abs(bz - fadj_bz)) {
            //- snap a to lowest
            if (az < fadj_az) {
              fadj->set_vertex_elevation(adj_a_ringi, adj_a_pi, az);
            }
            else
            {
              this->set_vertex_elevation(ringi, ai, fadj_az);
            }
          }
          else {
            //- snap b to lowest
            if (bz < fadj_bz) {
              fadj->set_vertex_elevation(adj_b_ringi, adj_b_pi, bz);
            }
            else {
              this->set_vertex_elevation(ringi, bi, fadj_bz);
            }
          }
        }
      }
    }
  }
}

void TopoFeature::construct_vertical_walls(const NodeColumn& nc) {
  //-- gather all rings
  std::vector<Ring2> therings;
  therings.push_back(_p2->outer());
  for (Ring2& iring : _p2->inners())
    therings.push_back(iring);

  //-- process each vertex of the polygon separately
  std::vector<int> anc, bnc;
  std::unordered_map<std::string, std::vector<int>>::const_iterator ncit;
  Point2 a, b;
  TopoFeature* fadj;
  int ringi = -1;
  for (Ring2& ring : therings) {
    ringi++;
    for (int ai = 0; ai < ring.size(); ai++) {
      //-- Point a
      a = ring[ai];
      //-- find Point b
      int bi;
      if (ai == (ring.size() - 1)) {
        b = ring.front();
        bi = 0;
      }
      else {
        b = ring[ai + 1];
        bi = ai + 1;
      }

      //-- find the adjacent polygon to segment ab (fadj)
      fadj = nullptr;
      int adj_a_ringi = 0;
      int adj_a_pi = 0;
      int adj_b_ringi = 0;
      int adj_b_pi = 0;
      for (auto& adj : *(_adjFeatures)) {
        if (adj->has_segment(b, a, adj_b_ringi, adj_b_pi, adj_a_ringi, adj_a_pi)) {
          fadj = adj;
          break;
        }
      }
      if (fadj == nullptr) {
        continue;
      }

      int az = this->get_vertex_elevation(ringi, ai);
      int bz = this->get_vertex_elevation(ringi, bi);
      int fadj_az = fadj->get_vertex_elevation(adj_a_ringi, adj_a_pi);
      int fadj_bz = fadj->get_vertex_elevation(adj_b_ringi, adj_b_pi);

      //-- check if there's a nc for either
      ncit = nc.find(gen_key_bucket(&a));
      if (ncit != nc.end()) {
        anc = ncit->second;
      }
      ncit = nc.find(gen_key_bucket(&b));
      if (ncit != nc.end()) {
        bnc = ncit->second;
      }
      if ((anc.empty() == true) && (bnc.empty() == true))
        continue;

      //-- Make exeption for bridges, they have vw's from bottom up, swap. Also skip if adjacent feature is bridge, vw is then created by bridge
      //-- Make exeption for buildings adjacent to water, they have vw's from bottom up.
      if ((this->get_class() == BRIDGE && this->get_top_level()) ||
        (this->get_class() == WATER && fadj->get_class() == BUILDING)) {
        //-- find the height of the vertex in the node column
        std::vector<int>::const_iterator sait, eait, sbit, ebit;
        sait = std::find(anc.begin(), anc.end(), az);
        eait = std::find(anc.begin(), anc.end(), fadj_az);
        sbit = std::find(bnc.begin(), bnc.end(), bz);
        ebit = std::find(bnc.begin(), bnc.end(), fadj_bz);

        // Set the top of the vertical wall to the bottom of the building
        if (this->get_class() == WATER && fadj->get_class() == BUILDING) {
          if (eait == anc.end()) {
            eait--;
            fadj_az = *eait;
          }
          if (ebit == bnc.end()) {
            ebit--;
            fadj_bz = *ebit;
          }
        }

        //-- iterate to triangulate
        while ((sbit != ebit) && (sbit != bnc.end()) && ((sbit + 1) != bnc.end())) {
          Point3 p;
          if (anc.size() == 0 || sait == anc.end()) {
            p = Point3(bg::get<0>(a), bg::get<1>(a), z_to_float(az));
            _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
          }
          else {
            p = Point3(bg::get<0>(a), bg::get<1>(a), z_to_float(*sait));
            _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
          }
          p = Point3(bg::get<0>(b), bg::get<1>(b), z_to_float(*sbit));
          _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
          sbit++;
          p = Point3(bg::get<0>(b), bg::get<1>(b), z_to_float(*sbit));
          _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
          Triangle t;
          int size = int(_vertices_vw.size());
          t.v0 = size - 1;
          t.v1 = size - 3;
          t.v2 = size - 2;
          _triangles_vw.push_back(t);
        }
        while (sait != eait && sait != anc.end() && (sait + 1) != anc.end()) {
          Point3 p;
          if (bnc.size() == 0 || ebit == bnc.end()) {
            p = Point3(bg::get<0>(b), bg::get<1>(b), z_to_float(bz));
            _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
          }
          else {
            p = Point3(bg::get<0>(b), bg::get<1>(b), z_to_float(*ebit));
            _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
          }
          p = Point3(bg::get<0>(a), bg::get<1>(a), z_to_float(*sait));
          _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
          sait++;
          p = Point3(bg::get<0>(a), bg::get<1>(a), z_to_float(*sait));
          _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
          Triangle t;
          int size = int(_vertices_vw.size());
          t.v0 = size - 1;
          t.v1 = size - 2;
          t.v2 = size - 3;
          _triangles_vw.push_back(t);
        }
      }
      if (this->get_class() == BRIDGE || fadj->get_class() == BRIDGE) {
        continue;
      }

      //-- check height differences: f > fadj for *both* Points a and b. 
      if (az < fadj_az || bz < fadj_bz) {
        continue;
      }
      if (az == fadj_az && bz == fadj_bz) {
        continue;
      }

      //-- find the height of the vertex in the node column
      std::vector<int>::const_iterator sait, eait, sbit, ebit;
      sait = std::find(anc.begin(), anc.end(), fadj_az);
      eait = std::find(anc.begin(), anc.end(), az);
      sbit = std::find(bnc.begin(), bnc.end(), fadj_bz);
      ebit = std::find(bnc.begin(), bnc.end(), bz);

      //-- iterate to triangulate
      while ((sbit != ebit) && (sbit != bnc.end()) && ((sbit + 1) != bnc.end())) {
        Point3 p;
        if (anc.size() == 0 || sait == anc.end()) {
          p = Point3(bg::get<0>(a), bg::get<1>(a), z_to_float(az));
          _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
        }
        else {
          p = Point3(bg::get<0>(a), bg::get<1>(a), z_to_float(*sait));
          _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
        }
        p = Point3(bg::get<0>(b), bg::get<1>(b), z_to_float(*sbit));
        _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
        sbit++;
        p = Point3(bg::get<0>(b), bg::get<1>(b), z_to_float(*sbit));
        _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
        Triangle t;
        int size = int(_vertices_vw.size());
        t.v0 = size - 2;
        t.v1 = size - 3;
        t.v2 = size - 1;
        _triangles_vw.push_back(t);
      }
      while (sait != eait && sait != anc.end() && (sait + 1) != anc.end()) {
        Point3 p;
        if (bnc.size() == 0 || ebit == bnc.end()) {
          p = Point3(bg::get<0>(b), bg::get<1>(b), z_to_float(bz));
          _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
        }
        else {
          p = Point3(bg::get<0>(b), bg::get<1>(b), z_to_float(*ebit));
          _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
        }
        p = Point3(bg::get<0>(a), bg::get<1>(a), z_to_float(*sait));
        _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
        sait++;
        p = Point3(bg::get<0>(a), bg::get<1>(a), z_to_float(*sait));
        _vertices_vw.push_back(std::make_pair(p, gen_key_bucket(&p)));
        Triangle t;
        int size = int(_vertices_vw.size());
        t.v0 = size - 3;
        t.v1 = size - 2;
        t.v2 = size - 1;
        _triangles_vw.push_back(t);
      }
    }
  }
}

bool TopoFeature::has_segment(const Point2& a, const Point2& b, int& aringi, int& api, int& bringi, int& bpi) {
  std::vector<int> ringis, pis;
  Point2 tmp;
  if (this->has_point2(a, ringis, pis) == true) {
    for (int k = 0; k < ringis.size(); k++) {
      int nextpi;
      tmp = this->get_next_point2_in_ring(ringis[k], pis[k], nextpi);
      if (sqr_distance(b, tmp) <= SQTOPODIST) {
        aringi = ringis[k];
        api = pis[k];
        bringi = ringis[k];
        bpi = nextpi;
        return true;
      }
    }
  }
  return false;
}

float TopoFeature::get_distance_to_boundaries(const Point2& p) {
  //-- collect the rings of the polygon
  std::vector<Ring2> therings;
  therings.push_back(_p2->outer());
  for (Ring2& iring : _p2->inners())
    therings.push_back(iring);

  //-- process each vertex of the polygon separately
  Point2 a, b;
  Segment2 s;
  int ringi = -1;
  double dmin = 99999;
  for (Ring2& ring : therings) {
    ringi++;
    for (int ai = 0; ai < ring.size(); ai++) {
      a = ring[ai];
      if (ai == (ring.size() - 1))
        b = ring.front();
      else
        b = ring[ai + 1];
      //-- calculate distance
      bg::set<0, 0>(s, bg::get<0>(a));
      bg::set<0, 1>(s, bg::get<1>(a));
      bg::set<1, 0>(s, bg::get<0>(b));
      bg::set<1, 1>(s, bg::get<1>(b));
      double d = bg::distance(p, s);
      if (d < dmin)
        dmin = d;
    }
  }
  return (float)dmin;
}

bool TopoFeature::has_point2(const Point2& p, std::vector<int>& ringis, std::vector<int>& pis) {
  std::vector<Ring2> rings;
  rings.push_back(_p2->outer());
  for (Ring2& iring : _p2->inners())
    rings.push_back(iring);

  bool re = false;
  int ringi = -1;
  for (Ring2& ring : rings) {
    ringi++;
    for (int i = 0; i < ring.size(); i++) {
      if (sqr_distance(p, ring[i]) <= SQTOPODIST) {
        ringis.push_back(ringi);
        pis.push_back(i);
        re = true;
        break;
      }
    }
  }
  return re;
}

bool TopoFeature::adjacent(Polygon2& poly) {
  std::vector<Ring2> rings1;
  rings1.push_back(_p2->outer());
  for (Ring2& iring : _p2->inners())
    rings1.push_back(iring);

  std::vector<Ring2> rings2;
  rings2.push_back(poly.outer());
  for (Ring2& iring : poly.inners())
    rings2.push_back(iring);

  for (Ring2& ring1 : rings1) {
    for (int pi1 = 0; pi1 < ring1.size(); pi1++) {
      for (Ring2& ring2 : rings2) {
        for (int pi2 = 0; pi2 < ring2.size(); pi2++) {
          if (sqr_distance(ring1[pi1], ring2[pi2]) <= SQTOPODIST) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

Point2 TopoFeature::get_point2(int ringi, int pi) {
  if (ringi == 0)
    return _p2->outer()[pi];
  else
    return _p2->inners()[ringi - 1][pi];
}

Point2 TopoFeature::get_next_point2_in_ring(int ringi, int i, int& pi) {
  Ring2 ring;
  if (ringi == 0)
    ring = _p2->outer();
  else
    ring = _p2->inners()[ringi - 1];

  if (i == (ring.size() - 1)) {
    pi = 0;
    return ring.front();
  }
  else {
    pi = i + 1;
    return ring[pi];
  }
}

bool TopoFeature::has_vertical_walls() {
  return _bVerticalWalls;
}

void TopoFeature::add_vertical_wall() {
  _bVerticalWalls = true;
}

int TopoFeature::get_vertex_elevation(int ringi, int pi) {
  return _p2z[ringi][pi];
}

int TopoFeature::get_vertex_elevation(const Point2& p) {
  std::vector<int> ringis, pis;
  has_point2(p, ringis, pis);
  return _p2z[ringis[0]][pis[0]];
}

void TopoFeature::set_vertex_elevation(int ringi, int pi, int z) {
  _p2z[ringi][pi] = z;
}

//-- used to collect all points linked to the polygon
//-- later all these values are used to lift the polygon (and put values in _p2z)
bool TopoFeature::assign_elevation_to_vertex(const Point2& p, double z, float radius) {
  double sqr_radius = radius * radius;
  int zcm = int(z * 100);

  int ringi = 0;
  Ring2& oring = _p2->outer();
  for (int i = 0; i < oring.size(); i++) {
    if (sqr_distance(p, oring[i]) <= sqr_radius)
      _lidarelevs[ringi][i].push_back(zcm);
  }
  ringi++;
  std::vector<Ring2>& irings = _p2->inners();
  for (Ring2& iring : irings) {
    for (int i = 0; i < iring.size(); i++) {
      if (sqr_distance(p, iring[i]) <= sqr_radius) {
        _lidarelevs[ringi][i].push_back(zcm);
      }
    }
    ringi++;
  }
  return true;
}

bool TopoFeature::within_range(const Point2& p, double radius) {
  if (point_in_polygon(p)) {
    return true;
  }  
  
  double sqr_radius = radius * radius;
  const Ring2& oring = _p2->outer();
  //-- point is within range of the polygon rings
  for (int i = 0; i < oring.size(); i++) {
    if (sqr_distance(p, oring[i]) <= sqr_radius) {
      return true;
    }
  }
  std::vector<Ring2>& irings = _p2->inners();
  for (Ring2& iring : irings) {
    for (int i = 0; i < iring.size(); i++) {
      if (sqr_distance(p, iring[i]) <= sqr_radius) {
        return true;
      }
    }
  }
  return false;
}

// based on http://stackoverflow.com/questions/217578/how-can-i-determine-whether-a-2d-point-is-within-a-polygon/2922778#2922778
bool TopoFeature::point_in_polygon(const Point2& p) {
  //test outer ring
  const Ring2& oring = _p2->outer();
  int nvert = oring.size();
  int i, j = 0;
  bool insideOuter = false;
  for (i = 0, j = nvert - 1; i < nvert; j = i++) {
    double py = p.y();
    if (((oring[i].y()>py) != (oring[j].y()>py)) &&
      (p.x() < (oring[j].x() - oring[i].x()) * (py - oring[i].y()) / (oring[j].y() - oring[i].y()) + oring[i].x()))
      insideOuter = !insideOuter;
  }
  if (insideOuter) {
    //test inner rings
    auto irings = _p2->inners();
    for (Ring2& iring : irings) {
      bool insideInner = false;
      int nvert = iring.size();
      int i, j = 0;
      for (i = 0, j = nvert - 1; i < nvert; j = i++) {
        double py = p.y();
        if (((iring[i].y() > py) != (iring[j].y() > py)) &&
          (p.x() < (iring[j].x() - iring[i].x()) * (py - iring[i].y()) / (iring[j].y() - iring[i].y()) + iring[i].x()))
          insideInner = !insideInner;
      }
      if (insideInner) {
        return false;
      }
    }
  }
  return insideOuter;
}

void TopoFeature::cleanup_lidarelevs() {
  _lidarelevs.clear();
  _lidarelevs.shrink_to_fit();
  _p2z.clear();
  _p2z.shrink_to_fit();
}

void TopoFeature::get_triangle_as_gml_surfacemember(std::wostream& of, Triangle& t, bool verticalwall) {
  of << "<gml:surfaceMember>";
  of << "<gml:Polygon>";
  of << "<gml:exterior>";
  of << "<gml:LinearRing>";
  if (verticalwall == false) {
    of << "<gml:posList>"
      << _vertices[t.v0].second << " "
      << _vertices[t.v1].second << " "
      << _vertices[t.v2].second << " "
      << _vertices[t.v0].second << "</gml:posList>";
  }
  else {
    of << "<gml:posList>"
      << _vertices_vw[t.v0].second << " "
      << _vertices_vw[t.v1].second << " "
      << _vertices_vw[t.v2].second << " "
      << _vertices_vw[t.v0].second << "</gml:posList>";
  }
  of << "</gml:LinearRing>";
  of << "</gml:exterior>";
  of << "</gml:Polygon>";
  of << "</gml:surfaceMember>";
}

void TopoFeature::get_floor_triangle_as_gml_surfacemember(std::wostream& of, Triangle& t, int baseheight) {
  of << "<gml:surfaceMember>";
  of << "<gml:Polygon>";
  of << "<gml:exterior>";
  of << "<gml:LinearRing>";

  std::stringstream ss;
  ss << std::fixed << std::setprecision(3);
  // replace z of the vertices with baseheight
  ss << "<gml:posList>"
    << _vertices[t.v0].second.substr(0, _vertices[t.v0].second.find_last_of(" ") + 1) << std::setprecision(2) << z_to_float(baseheight) << std::setprecision(3) << " "
    << _vertices[t.v2].second.substr(0, _vertices[t.v2].second.find_last_of(" ") + 1) << std::setprecision(2) << z_to_float(baseheight) << std::setprecision(3) << " "
    << _vertices[t.v1].second.substr(0, _vertices[t.v1].second.find_last_of(" ") + 1) << std::setprecision(2) << z_to_float(baseheight) << std::setprecision(3) << " "
    << _vertices[t.v0].second.substr(0, _vertices[t.v0].second.find_last_of(" ") + 1) << std::setprecision(2) << z_to_float(baseheight) << std::setprecision(3) << "</gml:posList>";

  of << ss.str();
  of << "</gml:LinearRing>";
  of << "</gml:exterior>";
  of << "</gml:Polygon>";
  of << "</gml:surfaceMember>";
}

void TopoFeature::get_triangle_as_gml_triangle(std::wostream& of, Triangle& t, bool verticalwall) {
  of << "<gml:Triangle>";
  of << "<gml:exterior>";
  of << "<gml:LinearRing>";
  if (verticalwall == false) {
    of << "<gml:posList>"
      << _vertices[t.v0].second << " "
      << _vertices[t.v1].second << " "
      << _vertices[t.v2].second << " "
      << _vertices[t.v0].second << "</gml:posList>";
  }
  else {
    of << _vertices_vw[t.v0].second << " "
      << _vertices_vw[t.v1].second << " "
      << _vertices_vw[t.v2].second << " "
      << _vertices_vw[t.v0].second << "</gml:posList>";
  }
  of << "</gml:LinearRing>";
  of << "</gml:exterior>";
  of << "</gml:Triangle>";
}

bool TopoFeature::get_attribute(std::string attributeName, std::string& attribute, std::string defaultValue)
{
  auto it = _attributes.find(attributeName);
  if (it != _attributes.end()) {
    attribute = (*it).second.second;
    if (!attribute.empty()) {
      // attribute is empty
      return true;
    }
    if (attribute.empty() && !defaultValue.empty()) {
      // attribute is empty but default is given
      attribute = defaultValue;
      return true;
    }
  }
  // attribute does not exist
  return false;
}

void TopoFeature::lift_all_boundary_vertices_same_height(int height) {
  int ringi = 0;
  Ring2& oring = _p2->outer();
  for (int i = 0; i < oring.size(); i++)
    _p2z[ringi][i] = height;
  ringi++;
  std::vector<Ring2>& irings = _p2->inners();
  for (Ring2& iring : irings) {
    for (int i = 0; i < iring.size(); i++)
      _p2z[ringi][i] = height;
    ringi++;
  }
}

void TopoFeature::add_adjacent_feature(TopoFeature* adjFeature) {
  _adjFeatures->push_back(adjFeature);
}

std::vector<TopoFeature*>* TopoFeature::get_adjacent_features() {
  return _adjFeatures;
}

void TopoFeature::lift_each_boundary_vertices(float percentile) {
  //-- assign value for each vertex based on percentile
  bool hasHeight = false;
  //-- gather all rings
  std::vector<Ring2> rings;
  rings.push_back(_p2->outer());
  for (Ring2& iring : _p2->inners())
    rings.push_back(iring);

  int ringi = -1;
  for (Ring2& ring : rings) {
    ringi++;
    for (int i = 0; i < ring.size(); i++) {
      std::vector<int> &l = _lidarelevs[ringi][i];
      if (l.empty() == true) {
        _p2z[ringi][i] = -9999;
      }
      else {
        std::nth_element(l.begin(), l.begin() + (l.size() * percentile), l.end());
        _p2z[ringi][i] = l[l.size() * percentile];
        hasHeight = true;
      }
    }
  }

  if (hasHeight) {// Skip setting heights if all heights are -9999
    //-- some vertices will have no values (no lidar point within tolerance thus)
    //-- assign them the closest height in its ring
    ringi = -1;
    for (Ring2& ring : rings) {
      ringi++;
      for (int i = 0; i < ring.size(); i++) {
        if (_p2z[ringi][i] == -9999) {
          // find closest previous or next vertex which does have a height and use it
          int next = -9999;
          int nextdistance = ring.size();
          int prev = -9999;
          int prevdistance = ring.size();
          for (int nexti = i; nexti < ring.size(); nexti++) {
            if (_p2z[ringi][nexti] != -9999) {
              next = _p2z[ringi][nexti];
              nextdistance = nexti - i;
              break;
            }
          }
          for (int previ = i; previ > 0; previ--) {
            if (_p2z[ringi][previ] != -9999) {
              prev = _p2z[ringi][previ];
              prevdistance = i- previ;
              break;
            }
          }
          if (nextdistance <= prevdistance) {
            _p2z[ringi][i] = next;
          }
          else if (prevdistance < nextdistance) {
            _p2z[ringi][i] = prev;
          }
        }
      }
    }
  }
}

//-------------------------------
//-------------------------------

Flat::Flat(char* wkt, std::string layername, AttributeMap attributes, std::string pid)
  : TopoFeature(wkt, layername, attributes, pid) {}

int Flat::get_number_vertices() {
  // return int(2 * _vertices.size());
  return (int(_vertices.size()) + int(_vertices_vw.size()));
}

bool Flat::add_elevation_point(Point2& p, double z, float radius, int lasclass, bool within) {
  // if within then a point must lay within the polygon, otherwise add
  if (!within || (within && point_in_polygon(p))) {
    if (within_range(p, radius)) {
      int zcm = int(z * 100);
      //-- 1. assign to polygon since within the threshold value (buffering of polygon)
      _zvaluesinside.push_back(zcm);
    }
  }
  return true;
}

int Flat::get_height() {
  return _height_top;
}

bool Flat::lift_percentile(float percentile) {
int z = -9999;
if (_zvaluesinside.empty() == false) {
  std::nth_element(_zvaluesinside.begin(), _zvaluesinside.begin() + (_zvaluesinside.size() * percentile), _zvaluesinside.end());
  z = _zvaluesinside[_zvaluesinside.size() * percentile];
}
this->_height_top = z;
this->lift_all_boundary_vertices_same_height(z);
return true;
}

void Flat::cleanup_elevations() {
  _zvaluesinside.clear();
  _zvaluesinside.shrink_to_fit();
  TopoFeature::cleanup_lidarelevs();
}

//-------------------------------
//-------------------------------

Boundary3D::Boundary3D(char* wkt, std::string layername, AttributeMap attributes, std::string pid)
  : TopoFeature(wkt, layername, attributes, pid) {
}

int Boundary3D::get_number_vertices() {
  return (int(_vertices.size()) + int(_vertices_vw.size()));
}

bool Boundary3D::add_elevation_point(Point2& p, double z, float radius, int lasclass, bool within) {
  // if within then a point must lay within the polygon, otherwise add
  if (!within || (within && point_in_polygon(p))) {
    assign_elevation_to_vertex(p, z, radius);
  }
  return true;
}

void Boundary3D::smooth_boundary(int passes) {
  //TODO: fix this or remove completely (tmp is not written back to r)
  std::vector<int> tmp;
  for (int p = 0; p < passes; p++) {
    for (auto& r : _p2z) {
      tmp.resize(r.size());
      tmp.front() = int((r[1] + r.back()) / 2);
      auto it = r.end();
      it -= 2;
      tmp.back() = int((r.front() + *it) / 2);
      for (int i = 1; i < (r.size() - 1); i++) {
        tmp[i] = int((r[i - 1] + r[i + 1]) / 2);
      }
    }
  }
}

void Boundary3D::detect_outliers(bool flatten){
  //-- gather all rings
  std::vector<Ring2> rings;
  rings.push_back(_p2->outer());
  for (Ring2& iring : _p2->inners())
    rings.push_back(iring);

  int ringi = -1;
  for (Ring2& ring : rings) {
    ringi++;

    // itterate only if >6 points in the ring or the LS will not work
    if (ring.size() > 6) {
      // find spikes in roads (due to misclassified lidar points) and fix by averaging between previous and next vertex.
      std::vector<int> idx;
      std::vector<double> x, y, z, coeffs;
      double x0 = ring[0].x();
      double y0 = ring[0].y();
      for (int i = 0; i < ring.size(); i++) {
        idx.push_back(i);
        x.push_back(ring[i].x());
        y.push_back(ring[i].y());
        z.push_back(_p2z[ringi][i]);
      }

      std::vector<double> xtmp = x, ytmp = y, ztmp = z;

      int niter = _p2z[ringi].size() - 6;
      std::vector<int> indices;
      double se = 0;
      for (int i = 0; i < niter; i++) {
        // Fit the model
        std::vector<double> correctedvalues;
        polyfit3d<double>(xtmp, ytmp, ztmp, x0, y0, coeffs, correctedvalues);
        std::vector<double> residuals, absResiduals;

        double sum = 0;
        for (int j = 0; j < correctedvalues.size(); j++) {
          absResiduals.push_back(abs(ztmp[j] - correctedvalues[j]));
          if (i == 0) {
            double z = ztmp[j] - correctedvalues[j];
            residuals.push_back(z);
            sum += z;
          }
        }
        if (i == 0) {
          int n = residuals.size();
          double mean = sum / n;

          double sq_sum = 0;
          std::for_each(residuals.begin(), residuals.end(), [&](const double res) {
            sq_sum += (res - mean) * (res - mean);
          });

          // Calculate standard deviation and 2 sigma
          double stdev = sqrt(sq_sum / (n - 1));
          se = 1.96 * stdev;
        }

        // Calculate the maximum residual
        auto max = std::max_element(absResiduals.begin(), absResiduals.end());
        // remove outlier if larger then 2*standard deviation
        if (*max > se) {
          int imax = max - absResiduals.begin();
          int vtx = idx[imax];

          // store the index of the vertex marked as an outlier
          indices.push_back(vtx);

          // remove the outlier from idx, xtmp, xtmp and xtmp arrays for next iteration
          idx.erase(idx.begin() + imax);
          xtmp.erase(xtmp.begin() + imax);
          ytmp.erase(ytmp.begin() + imax);
          ztmp.erase(ztmp.begin() + imax);
        }
        else {
          break;
        }
      }

      // get the new values based on the coeffs of the lase equation
      std::vector<double> correctedvalues;
      polyval3d<double>(x, y, x0, y0, coeffs, correctedvalues);
      if (flatten) {
        for (int i = 0; i < ring.size(); i++) {
          _p2z[ringi][i] = correctedvalues[i];
        }
      }
      else {
        for (int i : indices) {
          _p2z[ringi][i] = correctedvalues[i];
        }
      }
    }
  }
}

//-------------------------------
//-------------------------------

TIN::TIN(char* wkt, std::string layername, AttributeMap attributes, std::string pid, int simplification, double simplification_tinsimp, float innerbuffer)
  : TopoFeature(wkt, layername, attributes, pid) {
  _simplification = simplification;
  _simplification_tinsimp = simplification_tinsimp;
  _innerbuffer = innerbuffer;
}

int TIN::get_number_vertices() {
  return (int(_vertices.size()) + int(_vertices_vw.size()));
}

bool TIN::add_elevation_point(Point2& p, double z, float radius, int lasclass, bool within) {
  bool toadd = false;
  // if within then a point must lay within the polygon, otherwise add
  if (!within || (within && point_in_polygon(p))) {
    assign_elevation_to_vertex(p, z, radius);
  }
  if (_simplification <= 1)
    toadd = true;
  else {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, _simplification);
    if (dis(gen) == 1)
      toadd = true;
  }
  // Add the point to the lidar points if it is within the polygon and respecting the inner buffer size
  if (toadd && point_in_polygon(p) && (_innerbuffer == 0.0 || this->get_distance_to_boundaries(p) > _innerbuffer)) {
    _lidarpts.push_back(Point3(p.x(), p.y(), z));
  }
  return toadd;
}

void TIN::cleanup_elevations() {
  _lidarpts.clear();
  _lidarpts.shrink_to_fit();
  TopoFeature::cleanup_lidarelevs();
}

bool TIN::buildCDT() {
  return getCDT(_p2, _p2z, _vertices, _triangles, _lidarpts, _simplification_tinsimp);
}
