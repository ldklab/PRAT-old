#!/usr/bin/env bash
set -o errexit -o pipefail -o noclobber -o nounset

# Script to run gcov over C files and diff them
# with and without feature enabled.

# Option strings.
SHORT=d:
LONG=dir:

# Read args.
OPTS=$(getopt --options $SHORT --long $LONG --name "$0" -- "$@")

if [ $? != 0 ] ; then echo "Failed to parse options. Exiting." >&2 ; exit 1 ; fi

eval set -- "$OPTS"

# Initial values.
DIR="./src/"

makeCovFiles() {
	echo "Building project with llvm-gcov..."
	make || exit 1
	echo "Generating gcov files..."
	./mosquitto &
	last_pid=$!
	sleep 5s
	kill -KILL $last_pid
	gcov *.c || exit 1
	mkdir -p coverage_files
	mv *.c.gcov coverage_files/
	make clean
}

# Make this take path as arg later.
if [[ $PWD =~ "src" ]]
then
	makeCovFiles
else
	echo "Can't run in this directory. Exiting."
	exit 1
fi
