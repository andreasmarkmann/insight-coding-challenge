#!/usr/bin/env bash

## run script for running the rolling_median calculation with C++ files,

## compile sources with GNU C++ compiler g++ as specified by Makefile
## using C++11 standard, i.e. relies on g++ version >= 4.7
## divert compiler output to /dev/null to generate only program output
cd ./src/
make > /dev/null
cd ..
 
## execute my programs, with the input directory venmo_input and output the files in the directory venmo_output
if [ $? -eq 0 ] ; then
  ./src/rolling_median ./venmo_input/venmo-trans.txt ./venmo_output/output.txt
fi

