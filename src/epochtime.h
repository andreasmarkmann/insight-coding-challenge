#ifndef EPOCHTIME_H
#define EPOCHTIME_H

#define EPOCHTIME_FAIL 1
#define EPOCHTIME_SUCCESS 0

// string length of a UTC time input string
#define UTCTIMELEN 20

// maximum second allowed is 60 for leap seconds
#define MAXSEC 60

namespace epochtime {
  time_t epochParse(const char *UTCstring);
  void epochParseTest(const char *UTCstring);
}

// provide standardized shorthand namespace to save typing
namespace ept = epochtime;

#endif
