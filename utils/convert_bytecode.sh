#!/usr/bin/env bash

mkdir out/

for i in `find dds/ -type f -not -name '*.o' -maxdepth 1`
  do [[ $(file -b $i) = "LLVM IR bitcode" ]] && cp $i ${i/dds/out}.bc
done


# Simple script to change *.o -> *.bc
#for f in *.o; do
#  mv -- "$f" "${f%.o}.bc"
#done
