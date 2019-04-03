#!/bin/bash

DATA_DIR=../data
BIN_DIR=../bin
SRC_DIR=../src

TEXT_DATA=$DATA_DIR/rfc793_processed
VECTOR_DATA=$DATA_DIR/rfc793-vector.bin

if [ ! -e $VECTOR_DATA ]; then
  if [ ! -e $TEXT_DATA ]; then
		#sh ./create-text8-data.sh
		echo "Update create script for RFCs"
		exit 1
	fi
  echo -----------------------------------------------------------------------------------------------------
  echo -- Training vectors...
  time $BIN_DIR/word2vec -train $TEXT_DATA -output $VECTOR_DATA -cbow 1 -size 200 -window 8 -negative 25 -hs 0 -sample 1e-4 -threads 20 -binary 1 -iter 15
fi
