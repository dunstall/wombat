#!/bin/bash

# Depends: autoconf automake libtool

mkdir -p lib
pushd lib
  if [ -d "glog" ]; then
    exit 0
  fi

  git clone https://github.com/google/glog.git
  cd glog
  ./autogen.sh
  ./configure
  make -j 4
  make install
popd
