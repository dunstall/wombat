// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "log/log.h"
#include "server/event.h"

namespace wombat::broker {

class Partition {};

class LeaderPartition : public Partition {
 public:
  LeaderPartition(std::shared_ptr<server::EventQueue>,
                  std::shared_ptr<log::Log> log) {}
};

class ReplicaPartition : public Partition {
 public:
  ReplicaPartition(std::shared_ptr<server::EventQueue>,
                   std::shared_ptr<log::Log> log) {}
};

class Router {
 public:
  void Route(const server::Event& request);

  void AddPartition(std::unique_ptr<Partition> partition);
};

}  // namespace wombat::broker
