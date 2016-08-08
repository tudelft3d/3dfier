/*
 Copyright (c) 2015 Hugo Ledoux
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/
#ifndef Forest_h
#define Forest_h

#include "TopoFeature.h"

class Forest : public TIN
{
public:
  Forest (char *wkt, std::string pid, int simplification);
  bool          lift();
  bool          add_elevation_point(double x, double y, double z, float radius, LAS14Class lasclass, bool lastreturn);
  std::string   get_citygml();
  std::string   get_mtl();
  bool          get_shape(OGRLayer * layer);
  TopoClass     get_class();
  bool          is_hard();
};



#endif /* Forest_h */
