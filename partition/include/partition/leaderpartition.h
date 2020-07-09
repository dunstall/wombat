// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>

#include "log/log.h"
#include "partition/partition.h"
#include "server/event.h"

namespace wombat::broker {

class LeaderPartition : public Partition {
 public:
  LeaderPartition(std::shared_ptr<Handler> handler,
                  std::shared_ptr<log::Log> log) : Partition{0} {}

  void Handle(const server::Event& evt) override {}
};

}  // namespace wombat::broker
