#!/usr/bin/env bash

set -o errexit -o pipefail -o noclobber -o nounset

# Script to run gcov over C files and diff them
# with and without feature enabled.

# Option strings.
# TODO: add option for feature to toggle.
SHORT=d:f:
LONG=dir:feat:

# Read args.
OPTS=$(getopt --options $SHORT --long $LONG --name "$0" -- "$@")

if [ $? != 0 ] ; then echo "Failed to parse options. Exiting." >&2 ; exit 1 ; fi

eval set -- "$OPTS"

# Initial values.
DIR="."
FEAT="feat_not_specified"

while true; do
	case "$1" in
		#-h | --help ) usage ;;
		-d | --dir ) DIR="$2"; shift 2 ;;
		-f | --feat ) FEAT="$2"; shift 2 ;;
		-- ) shift; break ;;
		* ) break ;;
	esac
done

# This is for each fo the feature compilations. 
makeCovFiles() {
	echo "Building project with llvm-gcov..."
	make || exit 1
	echo "Generating gcov files..."
	./mosquitto & # Later this will invoke some comprehensive tests.
	last_pid=$!
	sleep 5s
	kill -KILL $last_pid
	gcov *.c || exit 1
	mkdir -p "coverage_files_$FEAT"
	mv *.c.gcov "coverage_files_$FEAT"
	make clean
}

# After running makeCovFiles twice (one for with and w/o feature)
# Check for files in coverage_files/ that match then diff them.
makeDiffs() {
	echo "TODO"
	exit 1
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
