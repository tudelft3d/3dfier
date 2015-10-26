//
//  Water.h
//
//  Created by Hugo Ledoux on 19/10/15.
//  Copyright © 2015 Hugo Ledoux. All rights reserved.
//

#ifndef Water_h
#define Water_h

#include "TopoFeature.h"


class Water : public Boundary3D
{
public:
  Water (Polygon2* p, std::string pid, std::string heightref);
  std::string get_obj_f(int offset);
  static std::string _heightref;
};


#endif /* Water_h */
