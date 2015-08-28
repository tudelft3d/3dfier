#ifndef INPUT_H
#define INPUT_H


// We are currently only including this to get the definition of polyhedraShell.
#include "Shell.h"
#include "Solid.h"
#include "pugixml.hpp"
#include <fstream>
#include <string>


class IOErrors {
  std::map<int, vector<std::string> >  _errors;
public:
  bool has_errors()
  {
    if (_errors.size() == 0)
      return false;
    else
      return true;
  }
  void add_error(int code, std::string info)
  {
    _errors[code].push_back(info);
    std::clog << "--> errors #" << code << " : " << info << std::endl;
  }
};

std::string   errorcode2description(int code, bool qie = false);
vector<Solid> readGMLfile(std::string &ifile, IOErrors& errs, double tol_snap, bool translatevertices = true);
Shell2*       readPolyfile(std::string &ifile, int shellid, IOErrors& errs, bool translatevertices = true);


#endif
