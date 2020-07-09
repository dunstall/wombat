// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>
#include <unordered_map>

#include "log/log.h"
#include "partition/partition.h"
#include "server/event.h"

namespace wombat::broker {

class Router {
 public:
  bool Route(const server::Event& evt);

  void AddPartition(std::unique_ptr<Partition> partition);

 private:
  std::unordered_map<uint32_t, std::unique_ptr<Partition>> partitions_;
};

}  // namespace wombat::broker
