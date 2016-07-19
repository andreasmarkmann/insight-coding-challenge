#ifndef STRINGUTILS_H
#define STRINGUTILS_H
#include <string>
#include <stdarg.h>

#define MAXSTRLEN 2048

namespace stringutils {
  std::string trim(const std::string& str,
                   const std::string& whitespace = " \t");
  std::string reduce(const std::string& str,
                     const std::string& whitespace = " \t",
                     const std::string& fill = " ");
  bool endAssert(std::string& str,
                 const std::string& endson);
  bool popQuoted(std::string& str,
                 std::string& quotedword);
  bool getSec(std::string& timestr, unsigned int& sec);
  void abortf(const char *msg, ...);
}

// provide standardized shorthand namespace to save typing
namespace stu = stringutils;

#endif
