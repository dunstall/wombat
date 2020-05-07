# Wombat
Distributed messaging system in Rust

[![Build Status](https://travis-ci.org/dunstall/wombat.svg?branch=develop)](https://travis-ci.org/dunstall/wombat)

# In progress
Release plan
* v0.1.0 (13/07/2020): Run a single consumer and producer against a single broker. System tests run locally.
* v0.2.0: Distribute consumer/client (requires coordination with ZooKeeper)
* v0.3.0: Distribute server (requires coordination with ZooKeeper)
* v0.4.0: Build CI/CD pipelines in AWS to run full test suite (unit tests and system tests) including benchmarking
* v0.5.0: Optimize (read papers etc, see https://cwiki.apache.org/confluence/display/KAFKA/Kafka+papers+and+presentations)
* v0.6.0: Build wombat as a service on AWS
