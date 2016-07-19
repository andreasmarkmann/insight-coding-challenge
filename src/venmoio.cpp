#include <iostream>     // std::cout
#include <fstream>      // std:ifstream std::ofstream
#include <cstring>      // strncpy
#include <limits>       // std::numeric_limits
#include <stdio.h>
#include <libgen.h>
#include <sys/stat.h>
#include "venmoio.h"
#include "venmodata.h"
#include "epochtime.h"
#include "stringutils.h"


// Constructor opens files and creates output directory of needed
venmoio::venmoio(const char* infname, const char* outfname) {
  // Input file must exist but output file directory may not exist
  // It will be only a single directory level, so no subdirs handled
  // Error may occur if directory exists but no need to catch that
  char myoutfname[MAXSTRLEN];
  strncpy(myoutfname, outfname, MAXSTRLEN);
  mkdir(dirname(myoutfname), 0755);
  infile.open(infname);
  outfile.open(outfname);
}

// Destructor closes files
venmoio::~venmoio() {
  infile.close();
  outfile.close();
}

// Parse a line and pass contents to data object
// expects one-line json container with "actor" "target" and
// "created_time" only in any order, with correct syntax,
// otherwise marks entry to be ignored
bool venmoio::parseLine(venmodata* vdt) {

#define IGNOREINPUT { vdt->supplied = vdt->FlagNone; return true; }

  // Reset venmodata to no content supplied
  vdt->supplied = vdt->FlagNone;

  // I want to load a limited number of characters to prevent buffer
  // overflow. istream.get requires a (char*) buffer to supply
  // a count limit for this functionality according to
  // http://www.cplusplus.com/reference/istream/istream/get/
  char Linebuf[MAXSTRLEN];

  // Result will contain success code, start with true,
  // load success variable with succesconstants code of individual
  // file operation.
  bool Result = true, Success;

  // get at most MAXSTRLEN characters per line
  // Note: Split this into two lines to force execution of operation
  // irrespective of current value of Result.
  Success = static_cast<bool>(infile.get(Linebuf, MAXSTRLEN));
  Result &= Success;

  if( Result ) {
    // get(string, count) reads up to and excluding newline, so
    // clear up to next newline from buffer and ignore contents
    Success = static_cast<bool>( infile.ignore(
      std::numeric_limits<std::streamsize>::max(), '\n') );
    // Instead, we can also read till the next newline
    //     Success = infile.get();
    // Note: Do not update Result with the Success value of this
    // operation in to allow final input line without newline.
    // Next get will fail and end program either way.


    // Trim whitespace and DOS file format carriage return if present
    std::string Input = stu::trim(Linebuf, " \t\f\v\n\r");

    // I will parse the input right-to-left to use convenient string
    // functions.

    // Last character must be curly brace, otherwise reject input.
    if( ! stu::endAssert(Input, "}") ) {
      IGNOREINPUT
    }

    // String holding Json name and content
    std::string InputName;
    std::string InputContent;

    // We are expecting venmodata::NNames ( = 3) pieces of information
    for(int NameCount = 0; NameCount < vdt->NNames; NameCount++) {
      // Whitespace and commas will both be trimmed from end
      bool hasContent = stu::popQuoted(Input, InputContent);
      // Is whitespace reduced content empty?
      if( 0 == InputContent.length() ) {
        IGNOREINPUT
      }
      bool hasColon = stu::endAssert(Input, ":");
      bool hasName = stu::popQuoted(Input, InputName);
      if( 0 == InputName.length() ) {
        IGNOREINPUT
      }
      if( ! (hasName && hasColon && hasContent) ) {
        IGNOREINPUT
      }

      // Note: replace this by hash lookup to support many Json
      // names but since we have only three types, a bunch of ifs
      // will suffice
      for(int NameOption = 0; NameOption < vdt->NNames; NameOption++) {
        if( vdt->Names[NameOption] == InputName ) {
          // Check if it has been supplied already.
          // If yes, ignore whole line.
          if( vdt->supplied & vdt->Flags[NameOption] ) {
            IGNOREINPUT
          }
          // mark this content as supplied
          vdt->supplied |= vdt->Flags[NameOption];
          // write content into appropriate content string
          *(vdt->Contents[NameOption]) = InputContent;
        }
      }
    }

    // Remaining character must be curly brace, otherwise
    // reject whole input line
    if( ! stu::endAssert(Input, "{") ) {
      IGNOREINPUT
    }
    if( 0 < Input.length() ) {
      IGNOREINPUT
    }

    // edges are non-directional, so swap if needed to obtain
    // lexicographically ordered actor <= target
    if( vdt->actor > vdt->target ) {
      std::string tempstr = vdt->actor;
      vdt->actor = vdt->target;
      vdt->target = tempstr;
    }

    if( ! stu::getSec(vdt->time, vdt->sec) ) {
      IGNOREINPUT
    }

    if( (vdt->epochtime = ept::epochParse(vdt->time.c_str())) < 0 ) {
      IGNOREINPUT
    }
  }

  return Result;
}

void venmoio::outStr(std::string str) {
  outfile << str;
}

// UNIT TESTING below
// ==================

// reads line from file into a buffer and writes it out to screen
bool venmoio::testLine() {
  // buffer for line, limited size for speed, will read with max length
  char linestr[MAXSTRLEN];

  // Result will contain success code, start with assuming success
  bool Result = true, Success;
  // get at most MAXSTRLEN characters per line to prevent buffer overflow
  Success = static_cast<bool>( infile.get(linestr, MAXSTRLEN) );
  Result &= Success;

  if( Result ) {
    std::cout << linestr << std::endl;

    // get(string, count) reads up to and excluding newline, so
    // clear up to next newline from buffer and ignore contents
    Success = static_cast<bool>( infile.get() );
  }

  return Result;
}
