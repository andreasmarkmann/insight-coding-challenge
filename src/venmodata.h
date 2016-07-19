#ifndef VENMODATA_H
#define VENMODATA_H
#include <string>
#include <fstream>
#include <time.h>       // time_t

class venmodata {
public:
  std::string actor, target, time;
  time_t epochtime;
  unsigned int sec, supplied;
  // declare last so it can point to actor, target, time
  std::string** Contents;

  // Number of supported Json tags
  static const unsigned int NNames;
  // Logical flags for which tags have been supplied
  static const unsigned int Flags[], FlagNone;
  static unsigned int FlagAll;
  // Strings containing supported Json tags
  static const char* Names[];

  // initializer list & constructor
  venmodata(std::string actor, std::string target, std::string time,
            time_t epochtime = 0, int sec = 0, int supplied = 0):
            actor(actor), target(target), time(time),
            epochtime(epochtime), sec(sec), supplied(supplied) {
              // initialize FlagAll to contain all Flags
              for(int ii = 0; ii < NNames; ii++) {
                FlagAll |= Flags[ii];
              }
              Contents = (std::string**)malloc(
                NNames * sizeof(std::string*) );
              // Points Contents array at object members same order as
              // Names, Flags to enable human-readable object syntax
              // Removing "this->" leads to segfault, for more, read
              // http://stackoverflow.com/questions/30258639/when-is-it-safe-to-call-this-in-constructor-and-destructor
              Contents[0] = &(this->time);
              Contents[1] = &(this->actor);
              Contents[2] = &(this->target);
            };
  ~venmodata();
  void cout();
};

#endif
