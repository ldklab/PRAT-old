#!/usr/bin/env bash

if [ "$EUID" -ne 0 ]; then
	echo "Script needs to be run as root."
	exit 1
fi

# Install all the dependencies required for PRAT.
apt-get install -y libssl-dev libwebsockets-dev uuid-dev 
