#!/usr/bin/env bash

set -o errexit -o pipefail -o noclobber -o nounset
trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

# Some shortcuts for printf colorization.
RED='\033[0;31m'
CYAN='\033[0;36m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

FEAT=$1

findMatches() {
	dir1="./cov_files_${FEAT^^}yes"
	dir2="./cov_files_${FEAT^^}no"
	# Find files that are in BOTH dirs.
	files=$(find "$dir1/" "$dir2/" -printf '%P\n' | sort | uniq -d)
	for f in $files
	do
		makeDiffs "$f"
	done
	exit 0
}

# Create the diffs of pertinent *.gcov files.
makeDiffs() {
	mkdir -p "diff_$FEAT"
	target=$1
	printf "Generating diff for ${CYAN} $target...${NC}\n"
	diff "./cov_files_${FEAT^^}yes/$target" "./cov_files_${FEAT^^}no/$target" > "diff_$FEAT/$target" &
	return
}

(findMatches)