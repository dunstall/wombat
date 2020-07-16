#!/bin/bash

GLOG_logtostderr=1 ./bazel-bin/broker/wombat-broker broker/broker/tests/data/WOMBATLEADER.conf
