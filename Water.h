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
  bool          threeDfy();
  std::string   get_citygml();
  std::string   get_obj_f(int offset);
  static std::string _heightref;
protected:
  void          make_boundary_horizontal(Polygon3 &p3);
};


#endif /* Water_h */
