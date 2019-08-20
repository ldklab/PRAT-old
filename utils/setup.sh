#!/usr/bin/env bash

set -o errexit -o pipefail -o noclobber -o nounset
trap "trap - SIGTERM" SIGINT SIGTERM EXIT

if [ "$EUID" -ne 0 ]; then
	echo "Script needs to be run as root."
	exit 1
fi

# Check distro running on
# for applicability of setup.
if [ -f /etc/os-release ]; then
  . /etc/os-release
  OS=$NAME
  VER=$VERSION_ID
  echo "OS: $OS"
  echo "VER: $VER"
fi

if [ $OS == "Ubuntu" ]; then
	# Install all the dependencies required for PRAT.
	apt-get install -y libssl-dev libwebsockets-dev uuid-dev libsystemd-dev
	apt-get install -y libcunit1 libcunit1-doc libcunit1-dev gnutls-dev libwrap0 libwrap0-dev
elif [ $OS == "Arch Linux" ]; then
	# Use pacman.
	# Check actual package names; these aren't set correctly.
	pacman -S libssl-dev libwebsockets-dev uuid-dev
else
	echo "Unsupported distribution. Exiting."
	exit 1
fi
