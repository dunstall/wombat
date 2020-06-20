#!/bin/bash

# Depends: autoconf automake libtool

mkdir -p lib
pushd lib
  git clone https://github.com/google/glog.git
  cd glog
  ./autogen.sh
  ./configure
  make -j 4
  sudo make install
popd
