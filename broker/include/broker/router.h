// Copyright 2020 Andrew Dunstall

#pragma once

#include <filesystem>
#include <memory>

#include "broker/conf.h"
#include "log/log.h"
#include "log/systemlog.h"
#include "server/responder.h"
#include "server/server.h"
#include "util/pollable.h"

namespace wombat::broker {

class Partition {};

class LeaderPartition : public Partition {
 public:
  LeaderPartition(std::shared_ptr<server::ResponseEventQueue>,
                  std::shared_ptr<log::Log> log) {}
};

class ReplicaPartition : public Partition {
 public:
  ReplicaPartition(std::shared_ptr<server::ResponseEventQueue>,
                   std::shared_ptr<log::Log> log) {}
};

class Router {
 public:
  void Route(const server::Event& request);

  void AddPartition(std::unique_ptr<Partition> partition);
};

}  // namespace wombat::broker
