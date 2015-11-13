//
//  Vegetation.h
//
//  Created by Hugo Ledoux on 19/10/15.
//  Copyright © 2015 Hugo Ledoux. All rights reserved.
//

#ifndef Vegetation_h
#define Vegetation_h

#include "TopoFeature.h"

class Vegetation : public TIN
{
public:
  Vegetation (Polygon2* p, std::string pid, int simplification);
  bool          lift();
  bool          buildCDT();
  std::string   get_citygml();
  std::string   get_obj_f(int offset);
  TopoClass     get_class();
};



#endif /* Vegetation_h */
