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
DIR="."

while true; do
	case "$1" in
		#-h | --help ) usage ;;
		-d | --dir ) DIR="$2"; shift 2 ;;
		-- ) shift; break ;;
		* ) break ;;
	esac
done

echo DIR=$DIR

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

usage() {
	echo "Usage $0 -d|--dir [source dir]"
	echo ""
	echo "[source dir] directory of source assets"
	echo ""
	exit 1
}

# I'll remove/fix later.
if [[ $DIR =~ "src" ]]
then
	# Run this guy in a subshell @ DIR.
	(cd $DIR; makeCovFiles)
else
	echo "Can't run in this directory. Exiting."
	exit 1
fi
