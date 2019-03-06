#!/bin/bash

# First, check which distro we're on || package manager installed.
if [ -f /etc/os-release ]; then
  . /etc/os-release
  OS=$NAME
  VER=$VERSION_ID
else
  echo "TEST"
fi

# TODO
# Need to add all the dependencies for CoreNLP.
# Get/build the corenlp-current jar and the models jar.

# Install lcov.
git clone https://github.com/linux-test-project/lcov.git ; cd lcov
make install

# Install CoreNLP.
git clone https://github.com/stanfordnlp/CoreNLP.git; cd CoreNLP
mvn package

# Get dependencies for angr.
#apt-get install python3-dev libffi-dev build-essential virtualenvwrapper

#mkvirtualenv --python=$(which python3) angr && pip install angr
