#!/usr/bin/env bash

# Simple script to change *.o -> *.bc
for f in *.o; do
  mv -- "$f" "${f%.o}.bc"
done
