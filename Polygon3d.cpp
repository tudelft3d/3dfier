
#include "Polygon3d.h"
#include "input.h"

Polygon3d::Polygon3d(Polygon2d* p, std::string id) {
  _id = id;
  _p2 = p;
}

Polygon3d::~Polygon3d() {
  // TODO: clear memory properly
  std::cout << "I am dead" << std::endl;
}

Box Polygon3d::get_bbox2d() {
  return bg::return_envelope<Box>(*_p2);
}

std::string Polygon3d::get_id() {
  return _id;
}

Polygon2d* Polygon3d::get_polygon2d() {
    return _p2;
}



//-------------------------------
//-------------------------------

Polygon3dBlock::Polygon3dBlock(Polygon2d* p, std::string id, std::string lifttype) : Polygon3d(p, id) 
{
  _lifttype = lifttype;
}

std::string Polygon3dBlock::get_lift_type() {
  return _lifttype;
}

std::string Polygon3dBlock::get_3d_citygml() {
  std::stringstream ss;
  ss << "<cityObjectMember>";
  ss << "<bldg:Building>";
  ss << "<bldg:measuredHeight uom=\"#m\">";
  ss << this->get_height();
  ss << "</bldg:measuredHeight>";
  ss << "<bldg:lod1Solid>";
  ss << "<gml:Solid>";
  ss << "<gml:exterior>";
  ss << "<gml:CompositeSurface>";
  //-- get floor
  ss << get_polygon_lifted_gml(this->_p2, 0, false);
  //-- get roof
  ss << get_polygon_lifted_gml(this->_p2, this->get_height(), true);
  //-- get the walls
  auto r = bg::exterior_ring(*(this->_p2));
  for (int i = 0; i < (r.size() - 1); i++) 
    ss << get_extruded_line_gml(&r[i], &r[i + 1], this->get_height(), 0, false);
  ss << "</gml:CompositeSurface>";
  ss << "</gml:exterior>";
  ss << "</gml:Solid>";
  ss << "</bldg:lod1Solid>";
  ss << "</bldg:Building>";
  ss << "</cityObjectMember>";
  return ss.str(); 
}

std::string Polygon3dBlock::get_3d_csv() {
  std::stringstream ss;
  ss << this->get_id() << ";" << this->get_height() << std::endl;
  return ss.str(); 
}

double Polygon3dBlock::get_height() {
  // TODO : return an error if no points
  if (_zvalues.size() == 0)
    return -999;
  std::string t = _lifttype.substr(_lifttype.find_first_of("-") + 1);
  if (t == "MAX") {
    double v = -99999;
    for (auto z : _zvalues) {
      if (z > v)
        v = z;
    }
    return v;
  }
  else if (t == "MIN") {
    double v = 99999;
    for (auto z : _zvalues) {
      if (z < v)
        v = z;
    }
    return v;
  }
  else if (t == "AVG") {
    double sum = 0.0;
    for (auto z : _zvalues) 
      sum += z;
    return (sum / _zvalues.size());
  }
  else if (t == "MEDIAN") {
    std::nth_element(_zvalues.begin(), _zvalues.begin() + (_zvalues.size() / 2), _zvalues.end());
    return _zvalues[_zvalues.size() / 2];
  }
  else {
    std::cout << "UNKNOWN HEIGHT" << std::endl;
  }
  return -9999;
}


bool Polygon3dBlock::add_elevation_point(double x, double y, double z) {
  _zvalues.push_back(z);
  return true;
}


//-------------------------------
//-------------------------------

Polygon3dBoundary::Polygon3dBoundary(Polygon2d* p, std::string id) : Polygon3d(p, id) 
{
}

std::string Polygon3dBoundary::get_lift_type() {
  return "BOUNDARY3D";
}

std::string Polygon3dBoundary::get_3d_citygml() {
  build_CDT();
  return ""; 
}

std::string Polygon3dBoundary::get_3d_csv() {
  return "EMPTY"; 
}


bool Polygon3dBoundary::add_elevation_point(double x, double y, double z) {
  _lidarpts.push_back(Point3d(x, y, z));
  return true;
}

bool Polygon3dBoundary::build_CDT() {
  std::cout << "---TRIANGULATE THE POLYGON---" << std::endl;

  auto oring = bg::exterior_ring(*_p2);
  auto irings = bg::interior_rings(*_p2);

  struct triangulateio in, out;
  in.numberofpoints = (oring.size() - 1);
  for (auto iring : irings)
    in.numberofpoints += (iring.size() - 1);
  in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));
  int counter = 0;
  //-- oring
  for (int i = 0; i < (oring.size() - 1); i++) {
    in.pointlist[counter++] = bg::get<0>(oring[i]);
    in.pointlist[counter++] = bg::get<1>(oring[i]);
  }
  //-- irings
  for (auto iring : irings) {
    for (int i = 0; i < (iring.size() - 1); i++) {
     in.pointlist[counter++] = bg::get<0>(iring[i]);
     in.pointlist[counter++] = bg::get<1>(iring[i]);
    }
  }
  in.numberofpointattributes = 0;
  in.pointmarkerlist = NULL;

  in.numberofsegments = in.numberofpoints;
  in.numberofholes = 0;
  in.numberofregions = 0;
  in.segmentlist = (int *) malloc(in.numberofsegments * 2 * sizeof(int));
  counter = 0;
  //-- oring
  int i;
  for (i = 0; i < (oring.size() - 2); i++) {
    in.segmentlist[counter++] = i;
    in.segmentlist[counter++] = (i + 1);
  }
  in.segmentlist[counter++] = i;
  in.segmentlist[counter++] = 0;
  //-- irings
  int start = (oring.size() - 1);
  for (auto iring : irings) {
    for (int i = 0; i < (iring.size() - 1); i++) {
      in.segmentlist[counter++] = (start + i);
      in.segmentlist[counter++] = (start + i + 1);
    }
    in.segmentlist[counter++] = (start + i);
    in.segmentlist[counter++] = (start);
    start += (iring.size() - 1);
  }
  in.segmentmarkerlist = NULL;
  
  
  out.pointlist = (REAL *) NULL;
  out.pointattributelist = (REAL *) NULL;
  out.pointmarkerlist = (int *) NULL;
  out.trianglelist = (int *) NULL;
  out.triangleattributelist = (REAL *) NULL;
  out.neighborlist = (int *) NULL;
  out.segmentlist = (int *) NULL;
  out.segmentmarkerlist = (int *) NULL;
  out.edgelist = (int *) NULL;
  out.edgemarkerlist = (int *) NULL;

  triangulate((char *)"pz", &in, &out, NULL);
  
  if (in.numberofpoints != out.numberofpoints)
    std::cout << "Points added." << std::endl;
  for (int i = 0; i < out.numberofpoints; i++) {
    std::cout << i << " : (" << out.pointlist[i * 2] << ", ";
    std::cout << out.pointlist[i * 2 + 1] << ", ";
  }

  std::cout << "# triangles : " << out.numberoftriangles << std::endl;
  for (int i = 0; i < out.numberoftriangles; i++) {
    std::cout << out.trianglelist[i * out.numberofcorners + 0] << "-";
    std::cout << out.trianglelist[i * out.numberofcorners + 1] << "-";
    std::cout << out.trianglelist[i * out.numberofcorners + 2] << std::endl;
  }

  free(in.pointlist);
  free(in.pointattributelist);
  free(in.pointmarkerlist);
  free(in.segmentlist);
  free(out.pointlist);
  free(out.pointattributelist);
  free(out.pointmarkerlist);
  free(out.trianglelist);
  free(out.triangleattributelist);
//  free(out.trianglearealist);
//  free(out.neighborlist);
  free(out.segmentlist);
  free(out.segmentmarkerlist);
  free(out.edgelist);
  free(out.edgemarkerlist);
  return true;
}


