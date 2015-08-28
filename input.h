#ifndef INPUT_H
#define INPUT_H


#include "definitions.h"
#include "Polygon3d.h"

bool read_gml_file(std::string ifile, std::vector<std::string>& layers, std::string idfield, std::vector<Polygon3d*>& lsPolys);

#endif
