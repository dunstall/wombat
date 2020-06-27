#!/bin/bash

# Requires cpplint (pip3 install cpplint)

cpplint --recursive --exclude=lib/ --filter=-build/c++11 .
