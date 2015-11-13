//
//  Building.h
//
//  Created by Hugo Ledoux on 19/10/15.
//  Copyright © 2015 Hugo Ledoux. All rights reserved.
//

#ifndef Building_h
#define Building_h

#include "TopoFeature.h"


class Building : public Block
{
public:
  Building (Polygon2* p, std::string pid, std::string heightref_top, std::string heightref_base);
  bool        lift();
  bool        buildCDT();
  std::string get_citygml();
  std::string get_csv();
  std::string get_obj_f(int offset);
  std::string get_obj_f_floor(int offset);
  TopoClass   get_class();
};


#endif /* Building_h */

