#include <iostream>
#include <time.h>
#include <string.h>
#include "epochtime.h"
#include "stringutils.h"


// Input UTCstring in format 2016-04-07T03:33:19Z and time zone format accepted
// Result points to struct tm type pointer of newly created time structure
// returns error flag

int parseUTCstring(const char *UTCstring, struct tm* Result) {
  char *ConversionResult =
    strptime(UTCstring, "%Y-%m-%dT%H:%M:%SZ", Result);

  if(NULL == ConversionResult) {
    return EPOCHTIME_FAIL;
  } else {
    return EPOCHTIME_SUCCESS;
  }
}

// Convert to secends since new year's 1970
time_t epochTime(struct tm* Time) {
  // Make sure we get UTC irrespective of our time zone
  return mktime(Time) + Time->tm_gmtoff;
}

// Convert to secends since new year's 1970,
// should coincide with mktime result at GMT
// if later than 1970, as per
// http://pubs.opengroup.org/onlinepubs/009695399/basedefs/xbd_chap04.html#tag_04_14
// Note tm_yday gets set automatically by strptime

time_t my_epochTime(struct tm* Time) {
  return Time->tm_sec + Time->tm_min*60 + Time->tm_hour*3600
    + Time->tm_yday*86400 + (Time->tm_year-70)*31536000 +
    ((Time->tm_year-69)/4)*86400 -
    ((Time->tm_year-1)/100)*86400 + ((Time->tm_year+299)/400)*86400;
}

time_t epochtime::epochParse(const char *UTCstring) {
  time_t Result = -1;
  std::string UTCtrim = stu::reduce(UTCstring, " \t", "");

  int ErrorCode = EPOCHTIME_FAIL;

  // safely check length of UTC time string and avoid buffer overflow
  if( UTCTIMELEN == strnlen(UTCtrim.c_str(), MAXSTRLEN) ) {
    struct tm* Time = new tm();
    ErrorCode = parseUTCstring(UTCtrim.c_str(), Time);
    if(EPOCHTIME_SUCCESS == ErrorCode) {
      // built-in method
      Result = epochTime(Time);
      // The following works for longer time periods
      // Result = my_epochTime(Time);
    }
    delete Time;
  }

  return Result;
}

// UNIT TESTING below
// ==================

// test function to be called from main program

void epochtime::epochParseTest(const char *UTCstring) {
  struct tm* Time = new tm();
  int ErrorCode = EPOCHTIME_FAIL;

  // safely check length of UTC time string and avoid buffer overflow
  // using GNU extension strnlen
  if( UTCTIMELEN == strnlen(UTCstring, MAXSTRLEN) ) {
    ErrorCode = parseUTCstring(UTCstring, Time);
  }

  std::cout << "Parsing time from string \""
    << UTCstring << "\" with error code " << ErrorCode << "." << std::endl;

  if(EPOCHTIME_SUCCESS == ErrorCode) {
    // Make sure we get UTC irrespective of our time zone
    std::cout << "Seconds since 1970: " << mktime(Time)+Time->tm_gmtoff << std::endl;
    std::cout << "My secs since 1970: " << my_epochTime(Time) << std::endl;
  } else {
    std::cout << "Error detected!" << std::endl;
  }

  delete Time;
}
