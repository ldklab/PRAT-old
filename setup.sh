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
	apt-get update
        apt-get install -y libssl-dev libwebsockets-dev uuid-dev libsystemd-dev maven git build-essential clang
        apt-get install -y libcunit1 libcunit1-doc libcunit1-dev gnutls-dev libwrap0 libwrap0-dev nasm
	
	# Install lcov.
	(git clone https://github.com/linux-test-project/lcov.git ; cd lcov; make install)

	# Install CoreNLP.
	#(git clone https://github.com/stanfordnlp/CoreNLP.git; cd CoreNLP; mvn package)

	# Get dependencies for angr.
	apt-get install -y python3-dev python3-pip libffi-dev build-essential virtualenvwrapper python3-tk
	#pip3 install -U -q h5py pyyaml tensorflow nltk spacy pandas paho-mqtt networkx matplotlib sklearn
	#pip3 install -U networkx matplotlib gensim annoy seaborn sklearn
	#python -m spacy download en_core_web_sm
	# pip install prodigy.wh1

        npm install -g diff2html-cli

	#mkvirtualenv --python=$(which python3) angr && pip install angr
	rm -rf lcov

elif [ $OS == "Arch Linux" ]; then
        # Use pacman.
        # Check actual package names; these aren't set correctly.
        pacman -S libssl-dev libwebsockets-dev uuid-dev
else
        echo "Unsupported distribution. Exiting."
        exit 1
fi

