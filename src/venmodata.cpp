#include <iostream>
#include <string>
#include "venmodata.h"

const unsigned int venmodata::NNames = 3;

const unsigned int venmodata::Flags[]  = {0x01, 0x02, 0x04};
const unsigned int venmodata::FlagNone = 0x00;
// Contains all Flag bits, gets initialized in constructor
// with non-exclusive OR, so here's a preview of the result
unsigned int venmodata::FlagAll  = 0x07;

const char* venmodata::Names[] = {"created_time", "actor", "target"};

venmodata::~venmodata() {
  free(Contents);
}

void venmodata::cout() {
    // evaluate data parsed into new object
    std::cout
             << venmodata::Names[0] << ": \"" << time
    << "\" " << venmodata::Names[1] << ": \"" << actor
    << "\" " << venmodata::Names[2] << ": \"" << target
    << "\" " << "Seconds" << ": \"" << sec
    << "\" " << "Epoch Time" << ": \"" << epochtime
    << "\" " << "Supplies" << ": \"" << supplied
    << "\"" << std::endl;
}
