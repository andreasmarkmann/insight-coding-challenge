#include "venmodata.h"
#include "venmoio.h"
#include "hashtable.h"
#include "graph.h"
#include "stringutils.h"


int main(int argc, char* argv[]) {

  // Expect two command line parameters, input and output filenames
  if(argc != 3) {
    stu::abortf("usage: %s <inputfile> <outputfile>\n", argv[0]);
  }

  // bash command line is limited size and we are using run script,
  // so command line parameters not sanitized
  // opens files and creates output directory of needed
  venmoio vio(argv[1], argv[2]);

  // Initialize data structures for processing
  Graph grp(&vio);

  // object that holds json data and flag showing which elements were
  // supplied, see venmodata.h
  venmodata vdt("", "", "");

  // vio.parseLine() reads a line and fills elements of vdt
  while( vio.parseLine(&vdt) ) {
    if( vdt.FlagAll == vdt.supplied ) {
      vdt.cout();
      grp.process(&vdt);
      grp.output();
      grp.test_output();
    }
  }

  return 0;
}
