#!/bin/bash
function abort {
  echo "produces Makefile input with dependencies extracted from source files."
  echo "usage: $0 <files>, where <files> are source files."
  exit 1
}

## if no argument given, print usage
if [ "x$1" = "x" ]
then
  abort
fi

echo "## $0 output follows:"
while [ ! -z "$1" ]
do
  infile="$1"
  filename=${infile##*/}
  filestem=${filename%.*}
  echo -n "$filestem.o:"
  echo -n " $filename"
  ## use regexp to pick out include file
  cat $infile | awk '{if(index($0,"#include \"")==1){ \
    printf(" %s", gensub(/#include "(.*)"/, "\\1", $0)) }}'
  echo
  shift
done
