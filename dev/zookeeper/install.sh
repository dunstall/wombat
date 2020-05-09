#!/bin/bash

set -e

# Install Java.
# https://kafka.apache.org/documentation/#java

tar -zxf jdk-8u241-linux-x64.tar.gz
mv jdk1.8.0_241 /usr/local/jdk1.8.0_241
export JAVA_HOME=/usr/local/jdk1.8.0_241

# Install zookeeper.
# https://kafka.apache.org/documentation/#zk

tar -zxf apache-zookeeper-3.5.7-bin.tar.gz
mv apache-zookeeper-3.5.7-bin /usr/local/zookeeper
cp zoo.cfg /usr/local/zookeeper/conf/zoo.cfg
