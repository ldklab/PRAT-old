#!/usr/bin/env bash

# First, check distro running on
# for applicability of setup.
if [ -f /etc/os-release ]; then
  . /etc/os-release
  OS=$NAME
  VER=$VERSION_ID
  echo "OS: $OS"
  echo "VER: $VER"
else
  echo "TEST"
fi

if [ "$EUID" -ne 0 ]; then
	echo "Script needs to be run as root."
	exit 1
fi

# Install all the dependencies required for PRAT.
apt-get install -y libssl-dev libwebsockets-dev uuid-dev 
