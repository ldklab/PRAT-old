#!/usr/bin/env bash

# Some shortcuts for printf colorization.
RED='\033[0;31m'
CYAN='\033[0;36m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Demo-1.
printf "${CYAN}[+] Running demo-1...${NC}\n\n\n"
docker run --rm prat:demo1
printf "\n\n\n\n"

read -n1 -rsp "Press space to continue..." key

if [ "$key" = '' ]; then
    printf "\n----------------------\n"
    printf "${CYAN}[+] Running demo-2-1...${NC}\n\n\n"
    docker run --rm prat:demo2-1
    printf "\n\n\n\n"
else
    echo "[-] Waiting..."
fi

read -n1 -rsp "Press space to continue..." key

if [ "$key" = '' ]; then
    printf "\n----------------------\n"
    printf "${CYAN}[+] Running demo-2...${NC}\n\n\n"
    docker run --rm prat:demo2 /bin/bash -c "ls -lh lib/libmosquitto.so.1 src/mosquitto && cloc src lib"
    printf "\n\n\n\n"
else
    echo "[-] Waiting..."
fi

read -n1 -rsp "Press space to continue..." key

if [ "$key" = '' ]; then
    printf "\n----------------------\n"
    printf "${CYAN}[+] Getting feature graphs...${NC}\n\n\n"
    xdot artifacts/graphs/callgraph_tls.dot && xdot artifacts/graphs/callgraph_rm_tls.dot
else
    echo "[-] Waiting..."
fi
