//
//  Terrain.h
//
//  Created by Hugo Ledoux on 19/10/15.
//  Copyright © 2015 Hugo Ledoux. All rights reserved.
//

#ifndef Terrain_h
#define Terrain_h

#include "TopoFeature.h"

class Terrain : public TIN
{
public:
  Terrain (Polygon2* p, std::string pid, int simplification);
  bool        lift();
  bool        buildCDT();
  std::string get_citygml();
  std::string get_obj_f(int offset);
};



#endif /* Terrain_h */
