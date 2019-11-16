#!/usr/bin/env bash

set -o errexit -o pipefail -o noclobber -o nounset
trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

# Script to run gcov over C files and diff them
# with and without feature enabled.

# Some shortcuts for printf colorization.
RED='\033[0;31m'
CYAN='\033[0;36m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Option strings.
SHORT=d:f:
LONG=dir:feat:

# Read args.
OPTS=$(getopt --options $SHORT --long $LONG --name "$0" -- "$@")

if [ $? != 0 ] ; then echo "Failed to parse options. Exiting." >&2 ; exit 1 ; fi

eval set -- "$OPTS"

# Populate a list of features using some NLP technique later.
featArr=("INC_BRIDGE_SUPPORT" "INC_DB_UPGRADE" "INC_MEMTRACK" "USE_LIBWRAP" "WITH_TLS" "WITH_THREADING" "WITH_DLT" "WITH_TLS_PSK" "WITH_PIC" "WITH_BRIDGE" "WITH_PERSISTENCE" "WITH_MEMORY_TRACKING" "WITH_SYS_TREE" "WITH_SYSTEMD" "WITH_SRV" "WITH_UUID" "WITH_WRAP" "WITH_STATIC_WEBSOCKETS" "WITH_WEBSOCKETS" "WITH_EC" "WITH_SOCKS" "WITH_EPOLL" "WITH_SHARED_LIBRARIES" "WITH_BUNDLED_DEPS")

# Initial values.
DIR="."
WORKDIR=$PWD
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

# Generate coverage files for Mosquitto.
makeMosquitto() {
	flag=$1

	printf "Building project with ${CYAN} ${FEAT^^}=$flag${NC}\n"
	printf "Running::${CYAN} make binary -j WITH_SHARED_LIBRARIES=yes WITH_COVERAGE=yes ${FEAT^^}=$flag${NC}\n"

	# Make the binary without the feature.
	make binary -j WITH_COVERAGE=yes WITH_SHARED_LIBRARIES=yes ${FEAT^^}=$flag || exit 1
	resloveDeps || exit 1 # Do this here because we need the shared lib.

	# Later this will invoke some comprehensive tests.
	printf "${GREEN} Generating gcov files...${NC}\n"
	mosquittoTests || exit 1

	(cd src; llvm-cov gcov *; cd -)
	(cd lib; llvm-cov gcov *; cd -)
	#(cd client; llvm-cov gcov *; cd -)

	mkdir -p "coverage_files_${FEAT^^}$flag"
	mv src/*.gcov "coverage_files_${FEAT^^}$flag" || true
	mv lib/*.gcov "coverage_files_${FEAT^^}$flag" || true
	#mv client/*.gcov "coverage_files_${FEAT^^}$flag" || true
	mv "coverage_files_${FEAT^^}$flag" $WORKDIR

	#printf "Running::${CYAN} make coverage${NC}\n"
	#make coverage
	#mv coverage.info $WORKDIR && mv out $WORKDIR

	printf "Running::${CYAN} make clean${NC}\n"
	make clean
}

# Generate the coverage files for non-Mosquitto projects.
generateCov() {
	# TODO.
	printf "${RED} 'generateCov()' feature not yet implemented. ${NC}\n"
	exit 1
}

resloveDeps() {
	if [[ ! -f "/usr/local/lib/libmosquitto.so.1" ]]
	then
		printf "${RED}libmosquitto.so.1 does not exist. Creating...${NC}\n"
		sudo cp "./lib/libmosquitto.so.1" "/usr/local/lib"
		sudo /sbin/ldconfig
	else
		printf "${GREEN} /usr/local/lib/libmosquitto.so.1 ${NC} already exists. Skipping.\n"
	fi

	# Conf test.
	if [[ ! -f "/etc/ld.so.conf.d/local.conf" ]]
	then
		sudo echo "/usr/local/lib" > "/etc/ld.so.conf.d/local.conf"
		sudo /sbin/ldconfig
	else
		printf "${GREEN} /etc/ld.so.conf.d/local.conf ${NC} already exists. Skipping.\n"
	fi
}

mosquittoTests() {
	printf "${CYAN}Running all unit tests...${NC}\n"
	# Broker is running; spawn clients
	# and run some tests.
	(cd test/unit; make test -j; cd -)

	./src/mosquitto &
	broker_pid=$!

	./client/mosquitto_sub -t 'test/topic1' -v &
	./client/mosquitto_pub -t 'test/topic1' -m 'hello, world'

	kill -KILL $broker_pid
	#exit 1
}

# After running makeCovFiles twice (one for with and w/o feature)
# Check for files in coverage_files/ that match then diff them.
findMatches() {
	dir1="./coverage_files_${FEAT^^}yes"
	dir2="./coverage_files_${FEAT^^}no"
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
	diff "./coverage_files_${FEAT^^}yes/$target" "./coverage_files_${FEAT^^}no/$target" > "diff_$FEAT/$target" &

	# Remove the empty gcov files. makes reading manually easier.
	#find ./"diff_$FEAT" -size 0 -print0 | xargs -0 rm --

	return
}

# Simple function to check if element is in array.
containsElement() {
	local e match="$1"
	shift
	for e; do [[ "$e" == "$match" ]] && return 0; done
	return 1
}

usage() {
	echo "Usage $0 -d|--dir [source dir]"
	echo ""
	echo "[source dir] directory of source assets"
	echo ""
	exit 1
}

# I'll remove/fix later.
if [[ $DIR =~ "mosquitto" ]] && containsElement "${FEAT^^}" "${featArr[@]}"; then
	# Run this guy in a subshell @ DIR.
	(cd $DIR; makeMosquitto yes && makeMosquitto no; cd $WORKDIR; findMatches)
elif [[ $DIR != "mosquitto" ]] && [ "$#" -ne 0 ]; then
	FEAT=$1
	(generateCov; findMatches)
else
	printf "${RED} Can't run in ${DIR} or ${FEAT^^} does not exist. Exiting.${NC}\n"
	exit 1
fi

