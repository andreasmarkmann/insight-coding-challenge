#!/bin/bash
dirname=$(pwd)
dirname=${dirname##*/}
target="${HOME}/Dropbox/16-ids"

make clean
echo "backing up $dirname to $target/$dirname/"
mkdir -p $target/$dirname
/usr/bin/rsync -avz * $target/$dirname/ \
--exclude '*.o' \
--exclude '*.o.bak' \

