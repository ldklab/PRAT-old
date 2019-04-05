#!/bin/bash

DATA_DIR=../data
BIN_DIR=../bin
SRC_DIR=../src

# Make this a cmd line arg later.
TEXT_DATA=$DATA_DIR/merged-rfc-1000
VECTOR_DATA=$DATA_DIR/merged-rfc-1000-skipgram-vector.bin

if [ ! -e $VECTOR_DATA ]; then
  if [ ! -e $TEXT_DATA ]; then
		#sh ./create-text8-data.sh
		echo "Update create script for RFCs"
		exit 1
	fi
  echo -----------------------------------------------------------------------------------------------------
  echo -- Training vectors...
  # Default size is 100. 200 caused RAM issues on laptop.
  time $BIN_DIR/word2vec -train $TEXT_DATA -output $VECTOR_DATA -cbow 0 -size 200 -window 8 -negative 25 -hs 0 -sample 1e-4 -threads 20 -binary 1 -iter 15
fi
