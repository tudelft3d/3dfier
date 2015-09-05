
#ifndef __3DFIER__Polygon3D__
#define __3DFIER__Polygon3D__

#include "definitions.h"


class Polygon3d
{
public:
  Polygon3d  (Polygon2d* p, std::string id);
  ~Polygon3d ();

  virtual Lift3DType    get_extrusion_type() = 0;
  virtual bool          add_elevation_point(double x, double y, double z) = 0;
  virtual std::string   get_3d_citygml() = 0;

  std::string   get_id();
  Polygon2d*    get_polygon2d();
  Box           get_bbox2d();

protected:
  Polygon2d*      _p2d;
  std::string     _id;
  int             _no_points;
  
 // vector<Point3d*> _lsPts3d;
};


class Polygon3d_block : public Polygon3d 
{
public:
  Polygon3d_block (Polygon2d* p, std::string id, BlockHeightRef heightref);
  bool            add_elevation_point(double x, double y, double z);
  Lift3DType      get_extrusion_type();
  std::string     get_3d_citygml();
  double          get_height();
  BlockHeightRef  get_height_reference();
private:
  std::vector<double> _zvalues;
  BlockHeightRef     _heightref; 
  // TODO : add a value/function to define a floor for each building too.
};




#endif /* defined(__boo__Polygon3D__) */
