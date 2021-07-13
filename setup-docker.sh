#!/bin/bash

set -o errexit -o pipefail -o noclobber -o nounset
trap "trap - SIGTERM" SIGINT SIGTERM EXIT

if [ "$EUID" -ne 0 ]; then
        echo "Script needs to be run as root."
        exit 1
fi

# First, check which distro we're on || package manager installed.
if [ -f /etc/os-release ]; then
  . /etc/os-release
  OS=$NAME
  VER=$VERSION_ID
  echo "OS: $OS"
  echo "VER: $VER"
fi

if [ $OS == "Ubuntu" ]; then
        # Install all the dependencies required for PRAT.
	apt -y update
        apt-get install -y libssl-dev libwebsockets-dev uuid-dev libsystemd-dev maven git build-essential clang python3 python3-pip llvm-10
        apt-get install -y libcunit1 libcunit1-doc libcunit1-dev gnutls-dev libwrap0 libwrap0-dev cloc --fix-missing
	pip3 install toml
        pip3 install pandas
        apt install libssl-dev
        # install klee
        apt-get -y update
        apt-get install -y build-essential curl libcap-dev cmake libncurses5-dev python2-minimal unzip libtcmalloc-minimal4 libgoogle-perftools-dev libsqlite3-dev doxygen
        apt-get install -y gcc-multilib g++-multilib
        apt-get install -y clang-9 llvm-9 llvm-9-dev llvm-9-tools
        apt-get install -y cmake bison flex libboost-all-dev python perl zlib1g-dev minisat
        # git clone https://github.com/stp/stp.git
        # cd stp
        # git checkout tags/2.3.3
        # mkdir build
        # cd build
        # cmake ..
        # make -j
        # make install
        # ulimit -s unlimited
        # cd ../../
        # git clone https://github.com/klee/klee.git
        # git clone https://github.com/klee/klee-uclibc.git  
        # cd klee-uclibc
        # ./configure --make-llvm-lib --with-llvm-config=/usr/bin/llvm-config-9
        # make -j2
        # cd ..
        # cd klee
        # mkdir build
        # cd build
        # cmake -DENABLE_POSIX_RUNTIME=ON -DENABLE_KLEE_UCLIBC=ON -DENABLE_UNIT_TESTS=OFF -DENABLE_SYSTEM_TESTS=OFF -DKLEE_UCLIBC_PATH=/PRAT/App/klee-uclibc -DLLVM_CONFIG_BINARY=/usr/bin/llvm-config-9 -DLLVMCC=/usr/bin/clang-9 -DLLVMCXX=/usr/bin/clang++-9 ..
        # make -j
        # make install
	# Install lcov.
	#(git clone https://github.com/linux-test-project/lcov.git ; cd lcov; make install)
elif [ $OS == "Arch Linux" ]; then
        # Use pacman.
        # Check actual package names; these aren't set correctly.
        pacman -S libssl-dev libwebsockets-dev uuid-dev
else
        echo "Unsupported distribution. Exiting."
        exit 1
fi

