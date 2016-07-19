#include <iostream>     // std::cout
#include <string>
#include <cstdlib>
#include <stdarg.h>
#include <stdio.h>
#include "epochtime.h"
#include "stringutils.h"


// portable string trimming (functions trim and reduce) adapted from
// http://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string

// Trim characters listed in whitespace from string str
// and return trimmed substr.

std::string stringutils::trim(const std::string& str,
                              const std::string& whitespace) {
  const size_t strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string::npos) {
    return ""; // no content
  }

  const size_t strEnd = str.find_last_not_of(whitespace);
  const size_t strRange = strEnd - strBegin + 1;

  return str.substr(strBegin, strRange);
}

// Reduce string str by trimming whitespace and replacing multiple
// occurences of whitespace characters inside str by single
// instance of fill, returns string with these replacements made.

std::string stringutils::reduce(const std::string& str,
                                const std::string& whitespace,
                                const std::string& fill) {
  // trim first
  std::string result = stu::trim(str, whitespace);

  // replace sub ranges
  size_t beginSpace = result.find_first_of(whitespace);
  while (beginSpace != std::string::npos) {
    const size_t endSpace = result.find_first_not_of(whitespace, beginSpace);
    const size_t range = endSpace - beginSpace;

    result.replace(beginSpace, range, fill);

    const size_t newStart = beginSpace + fill.length();
    beginSpace = result.find_first_of(whitespace, newStart);
  }

  return result;
}

// Remove expected next character from start of string if matching.
// Returns true if matched and successfully removed, false otherwise.
bool stringutils::endAssert(std::string& str,
                            const std::string& endson) {
  size_t endMatch = str.find_last_of(endson);
  if( endMatch != (str.length() - 1) ) {
    return false;
  } else {
    // Remove last character if matched
    // C++11 function supported by GNU g++, may be replaced by
    // str.resize(str.length() - 1);
    str.pop_back();
    return true;
  }
}

// Get quoted content "word" from end of string, and crop it off.
bool stringutils::popQuoted(std::string& str,
                            std::string& quotedword) {
  // Trim any whitespace or commas from end
  while( stu::endAssert(str, ", \t\f\v\n\r") ) {
  }

  // Assert that word is quoted
  if( ! stu::endAssert(str, "\"") ) {
    return false;
  }

  // Content word is between quotes
  const size_t beginQuote = str.find_last_of("\"");

  // If no opening quote found, exit unsuccessfully
  if( std::string::npos == beginQuote ) {
    return false;
  }
  // Get content between quotes
  std::string Quoted = str.substr(beginQuote + 1);
  // Trim and reduce white space in Quoted string
  quotedword = stu::reduce(Quoted, " \t", " ");

  // Did find opening quote, remove up to and including it
  str.resize(beginQuote);

  // Trim any further whitespace from end
  while( stu::endAssert(str, " \t\f\v\n\r") ) {
  }

  // If we made it here, everything went well
  return true;
}

// Get seconds after the minute from UTC time string
bool stringutils::getSec(std::string& timestr, unsigned int& sec) {
  // operate on copy of time string in order to not chop seconds off
  std::string mytime = timestr;

  // time string is already comma separated, expect T-Z format
  if( ! stu::endAssert(mytime, "Z") ) {
    return false;
  }
  // Seconds are in front of colon
  const size_t beginSec = mytime.find_last_of(":");

  // If no colon found, exit unsuccessfully
  if( std::string::npos == beginSec ) {
    return false;
  }

  // Get second content
  std::string secString = mytime.substr(beginSec + 1);

  // Use safe string conversion for input C++11 function stoi
  // For safe conversion of string to integer pre C++11, see
  // http://stackoverflow.com/a/6154614/1890916
  int mysec = std::stoi(secString);

  // allow sec == 60 for leap seconds
  if( mysec < 0 || mysec > MAXSEC ) {
    return false;
  }
  // If leap second, bring down to 59 just like time.h
  if( MAXSEC == mysec ) {
    mysec = MAXSEC - 1;
  }
  sec = (unsigned int)mysec;
  return true;
}

// Abort the program with a customizable error message, C style

void stringutils::abortf(const char *msg, ...) {
  va_list argp;

  va_start(argp, msg);
  vfprintf(stderr, msg, argp);
  va_end(argp);
  exit(EXIT_FAILURE);
}

// // replacement implementation for GNU extension strnlen from
// // http://stackoverflow.com/questions/5935413/is-there-a-safe-version-of-strlen/20736856#20736856
//
// size_t strnlen(const char *str, size_t max_len) {
//   const char* end = (const char *)memchr(str, '\0', max_len);
//   if (end == NULL)
//     return max_len;
//   else
//     return end - str;
// }
