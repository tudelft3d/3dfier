//
//  Polygon3d.h
//  boo
//
//  Created by Hugo Ledoux on 28/08/15.
//  Copyright (c) 2015 Hugo Ledoux. All rights reserved.
//

#ifndef __3DFIER__Polygon3D__
#define __3DFIER__Polygon3D__

#include "definitions.h"


class Polygon3d
{
public:
  Polygon3d  ();
  Polygon3d  (Polygon2d* p, ExtrusionType type, std::string id);
  ~Polygon3d ();

  Box get_bbox2d();
  std::string get_id();
     
private:
  Polygon2d*    _p2d;
  std::string   _id;
  ExtrusionType _type;
};


#endif /* defined(__boo__Polygon3D__) */
