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
sleep 5

printf "\n----------------------\n"
printf "${CYAN}[+] Running demo-2...${NC}\n\n\n"
docker run --rm prat:demo2
printf "\n\n\n\n"
sleep 5

printf "\n----------------------\n"
printf "${CYAN}[+] Running demo-2-1...${NC}\n\n\n"
docker run --rm prat:demo2-1 /bin/bash -c "cloc src lib"
