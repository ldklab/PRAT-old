#!/usr/bin/env bash

# Container for demo-1.
echo "[+] Building container for demo-1..."
docker build -t prat:demo1 -f docker/demo1/Dockerfile .

# Container for demo-2.
echo "[+] Building container for demo-2..."
docker build -t prat:demo2 -f docker/demo2/Dockerfile .

# Container for demo-2-1.
echo "[+] Building container for demo-2-1..."
docker build -t prat:demo2-1 -f docker/demo2-1/Dockerfile .

# Container for demo-3.
echo "[+] Building container for demo-3..."
docker build -t prat:demo3 -f docker/demo3/Dockerfile .
