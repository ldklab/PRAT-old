#!/usr/bin/env bash

set -o errexit -o pipefail -o noclobber -o nounset

# Script to run gcov over C files and diff them
# with and without feature enabled.

# Some shortcuts for printf colorization.
RED='\033[0;31m'
CYAN='\033[0;36m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Option strings.
# TODO: add option for feature to toggle.
SHORT=d:f:
LONG=dir:feat:

# Read args.
OPTS=$(getopt --options $SHORT --long $LONG --name "$0" -- "$@")

if [ $? != 0 ] ; then echo "Failed to parse options. Exiting." >&2 ; exit 1 ; fi

eval set -- "$OPTS"

# Populate a list of features using some NLP technique later.
feat_arr=(TLS THREADING BRIDGE PERSISTENCE MEMORY_TRACKING SYS_TREE SYSTEMD SRV UUID WEBSOCKETS)

# Initial values.
DIR="."
FEAT=""

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
	flag=$1
	printf "Building project with ${CYAN} ${FEAT^^}=$flag${NC}\n"
	# Make the binary without the feature.
	make WITH_${FEAT^^}=$flag || exit 1
	printf "${GREEN} Generating gcov files...${NC}\n"
	./mosquitto & # Later this will invoke some comprehensive tests.
	last_pid=$!
	sleep 5s
	kill -KILL $last_pid
	gcov *.c || exit 1
	mkdir -p "coverage_files_${FEAT^^}$flag"
	mv *.c.gcov "coverage_files_${FEAT^^}$flag"
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
	(cd $DIR; makeCovFiles yes && makeCovFiles no)
else
	echo "Can't run in this directory. Exiting."
	exit 1
fi
