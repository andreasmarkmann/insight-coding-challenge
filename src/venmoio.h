#ifndef VENMOIO_H
#define VENMOIO_H
#include <iostream>
#include <fstream>
#include "venmodata.h"

class venmoio {
protected:
  std::ifstream infile;
  std::ofstream outfile;
public:
  venmoio(const char* infname, const char* outfname);
  ~venmoio();
  bool parseLine(venmodata* vdt);
  void outStr(std::string str);
  bool testLine();
};

#endif
