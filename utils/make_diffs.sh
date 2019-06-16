#!/usr/bin/env bash

# Script to run gcov over C files and diff them
# with and without feature enabled.

makeCovFiles() {
	echo "Building project with llvm-gcov..."
	make
	echo "Generating gcov files..."
	./mosquitto
	gcov *.c
	make clean
}

if [[ $PWD =~ "src" ]]
then
	makeCovFiles
else
	echo "Can't run in this directory. Exiting"
	exit 1
fi