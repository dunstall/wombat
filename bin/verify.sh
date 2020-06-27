#!/bin/bash

./bin/exec.py clean
./bin/exec.py build
./bin/exec.py unittest
./bin/exec.py integrationtest
./bin/staticanalysis.sh
