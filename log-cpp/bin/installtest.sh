#!/bin/bash

mkdir -p lib
pushd lib
  git clone https://github.com/google/googletest.git
  cd googletest
  mkdir build
  cd build
  cmake ..
  make
popd
