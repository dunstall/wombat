#!/bin/bash

find . -name "*.h" | xargs clang-format -i --style=Google
find . -name "*.cc" | xargs clang-format -i --style=Google
buildifier -r .
