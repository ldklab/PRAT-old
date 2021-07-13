#!/usr/bin/env bash

# TODO: This should probably be ported to a Python program or
# something else that provides a cleaner interface to the user
# without bash versioning issues.

set -o errexit -o pipefail -o noclobber -o nounset
# Following trap caused an error. Fix later.
#trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

# Script to run gcov over C files and diff them
# with and without feature enabled.

# Some shortcuts for printf colorization.
RED='\033[0;31m'
CYAN='\033[0;36m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Option strings.
SHORT=d:f:h:
LONG=dir:feat:help:

# Read args.
OPTS=$(getopt --options $SHORT --long $LONG --name "$0" -- "$@")

usage() {
cat <<HELP_USAGE
$0 -d <dir> -f <feature>

-d Directory containing project source code
-f Feature to target for removal
HELP_USAGE
}

if [ "$#" -eq 0 ] ; then usage ; exit 1 ; fi

if [ $? != 0 ] ; then echo "Failed to parse options. Exiting." >&2 ; exit 1 ; fi

eval set -- "$OPTS"

# Populate a list of features using some NLP technique later.
featArr=("INC_BRIDGE_SUPPORT" "INC_DB_UPGRADE" "INC_MEMTRACK" "USE_LIBWRAP" "WITH_TLS" "WITH_THREADING" "WITH_DLT" "WITH_TLS_PSK" "WITH_PIC" "WITH_BRIDGE" "WITH_PERSISTENCE" "WITH_MEMORY_TRACKING" "WITH_SYS_TREE" "WITH_SYSTEMD" "WITH_SRV" "WITH_UUID" "WITH_WRAP" "WITH_STATIC_WEBSOCKETS" "WITH_WEBSOCKETS" "WITH_EC" "WITH_SOCKS" "WITH_EPOLL" "WITH_SHARED_LIBRARIES" "WITH_BUNDLED_DEPS")
openDDSFeatures=$(cat <<- EOM
--[no-]built-in-topics          Built-in Topics (yes)
--[no-]content-subscription     Content-Subscription Profile (yes)
--[no-]content-filtered-topic   ContentFilteredTopic (CS Profile) (yes)
--[no-]multi-topic              MultiTopic (CS Profile) (yes)
--[no-]query-condition          QueryCondition (CS Profile) (yes)
--[no-]ownership-profile        Ownership Profile (yes)
--[no-]ownership-kind-exclusive Exclusive Ownership (Ownership Profile) (yes)
--[no-]object-model-profile     Object Model Profile (yes)
--[no-]persistence-profile      Persistence Profile (yes)
--safety-profile[=VAL]          Safety Profile: base or extended (none)
EOM
)

# Initial values.
DIR="."
WORKDIR=$PWD
FEAT=""

while true; do
	case "$1" in
		-h | --help ) usage ;;
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
	#mosquittoTests || exit 1

	(cd src; llvm-cov-9 gcov *; cd -)
	(cd lib; llvm-cov-9 gcov *; cd -)
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

# Generate the coverage files for CMake-based projects.
generateCovCM() {
	flag=$1
	baseFeat=$2

	if [ $flag == ON ]; then
		feat="-D$baseFeat:BOOL=ON"
	else
		feat="-D$baseFeat:BOOL=OFF"
	fi

	printf "Running::${CYAN} cmake $feat .. ${NC}\n"

	mkdir -p "coverage_files_$baseFeat$flag" || true
	mv src/*.gcov "coverage_files_$baseFeat$flag" || true

	mv "coverage_files_$baseFeat$flag" $WORKDIR

	exit 1
}

# Generate the coverage files for FFmpeg.
makeFFmpeg() {
	flag=$1
	baseFeat=$2

	# Hard-code a feature for now; this whole part may be unneeded later.
	if [ $flag == yes ]; then
		# This project might just need disabled, or nothing.
		feat=""
	else
		feat="--disable-$baseFeat"
	fi

	printf "Running::${CYAN} ./configure --toolchain=gcov $feat ${NC}\n"

	# Prep the build.
	./configure --toolchain=gcov $feat || exit 1

	printf "Running::${CYAN} make -j2 ${NC}\n"
	make -j || exit 1

	# Later this will invoke some comprehensive tests.
	printf "${GREEN} Generating gcov files...${NC}\n"
	if ! command -v gcov &> /dev/null
	then
		# Just testing on filters for now.
		(cd libavfilter; gcov *; cd ~)
	else
		printf "[-] ${RED}gcov not available. Exiting${NC}\n"
		exit 1
	fi

	mkdir -p "coverage_files_$feat$flag" || true
	mv libavfilter/*.gcov "coverage_files_$feat$flag" || true

	mv "coverage_files_$feat$flag" $WORKDIR

	exit 1
}

# Generate the coverage files for autotool-based projects.
generateCovAT() {
	flag=$1
	baseFeat=$2

	# Hard-code a feature for now; this whole part may be unneeded later.
	if [ $flag == yes ]; then
		feat="--$baseFeat"
	else
		feat="--no-$baseFeat"
	fi

	printf "${RED} 'generateCov()' feature is only working for OpenDDS right now. ${NC}\n"
	printf "Available features for OpenDDS:\n $openDDSFeatures \n"
	printf "Running::${CYAN} ./configure --no-tests $feat ${NC}\n"

	# Prep the build.
	./configure --no-tests $feat || exit 1
	sh setenv.sh || exit 1

	printf "Running::${CYAN} make -j ${NC}\n"
	make -j || exit 1

	# Later this will invoke some comprehensive tests.
	printf "${GREEN} Generating gcov files...${NC}\n"
	if ! command -v llvm-cov &> /dev/null
	then
		(cd dds; llvm-cov-10 gcov *; cd ~)
	else
		(cd dds; llvm-cov gcov *; cd -)
	fi

	mkdir -p "coverage_files_$feat$flag" || true
	mv dds/*.gcov "coverage_files_$feat$flag" || true

	mv "coverage_files_$feat$flag" $WORKDIR

	exit 1
}

# This sometimes does need to be re-run, so we need a better check.
# Otherwise, we may get a version of the shared lib mismatch, which
# makes tests fail.
resloveDeps() {
	if [[ ! -f "/usr/local/lib/libmosquitto.so.1" ]]
	then
		printf "${RED}libmosquitto.so.1 does not exist. Creating...${NC}\n"
		sudo cp "./lib/libmosquitto.so.1" "/usr/local/lib"
		sudo /sbin/ldconfig
	else
		printf "${GREEN} /usr/local/lib/libmosquitto.so.1 ${NC} already exists. Updating.\n"
		sudo cp "./lib/libmosquitto.so.1" "/usr/local/lib"
		sudo /sbin/ldconfig
	fi

	# Conf test.
	if [[ ! -f "/etc/ld.so.conf.d/local.conf" ]]
	then
		echo "/usr/local/lib" > "/etc/ld.so.conf.d/local.conf"
		sudo /sbin/ldconfig
	else
		printf "${GREEN} /etc/ld.so.conf.d/local.conf ${NC} already exists. Skipping.\n"
	fi
}

mosquittoTests() {
	printf "${CYAN}Running all unit tests...${NC}\n"
	# Broker is running; spawn clients
	# and run some tests.
	#(cd test; make test -j; cd -) # This might change back to test/unit dir.

	# This is where I will call the symbolically-generated tests
	# to get more precise coverage for removal.

	./src/mosquitto &
	broker_pid=$!

	./client/mosquitto_sub -t 'test/topic1' -v &
	./client/mosquitto_pub -t 'test/topic1' -m 'hello, world'

	kill -KILL $broker_pid || exit 1
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
	find ./"diff_$FEAT" -size 0 -print0 | xargs -0 -r rm --

	return
}

# Simple function to check if element is in array.
containsElement() {
	local e match="$1"
	shift
	for e; do [[ "$e" == "$match" ]] && return 0; done
	return 1
}

# TODO: make this handle arguments cleanly. Right now it's buggy.
if [[ $DIR =~ "mosquitto" ]] && containsElement "${FEAT^^}" "${featArr[@]}"; then
	# Run this guy in a subshell @ DIR.
	(cd $DIR; makeMosquitto yes && makeMosquitto no; cd $WORKDIR; findMatches)
	# Now we want to run the perl script to make diffs and show # lines to remove.
	printf "${CYAN}Extracting feature locations for removal...${NC}\n"
	sleep 3
	./extract_features.pl "diff_$FEAT/"
elif [[ $DIR =~ "FFmpeg" ]] && [ "$#" -ne 0 ]; then
	FEAT=$1
	(cd $DIR; makeFFmpeg yes $FEAT && makeFFmpeg no $FEAT; findMatches)
elif [[ $DIR =~ "DDS" ]] && [ "$#" -ne 0 ]; then
	# This is hard-coded for now. Will fix later.
	DIR=$1
	FEAT=$2
	(cd $DIR; generateCovAT yes $FEAT&& generateCovAT no $FEAT; findMatches)
# Fix this later, or wait till Python port.
#elif [[ $DIR != "DDS" ]] && [ "$#" -ne 0 ]; then
	# Other CMake-based ones.
#	DIR=$1
#	FEAT=$2
#	(cd $DIR; generateCovCM ON $FEAT&& generateCovCM OFF $FEAT; findMatches)
else
	printf "${RED} Can't run in ${DIR} or ${FEAT^^} does not exist. Exiting.${NC}\n"
	exit 1
fi
