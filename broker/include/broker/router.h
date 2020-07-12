// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>
#include <unordered_map>

#include "event/event.h"
#include "log/log.h"
#include "partition/partition.h"

namespace wombat::broker {

class Router {
 public:
  bool Route(const Event& evt);

  void AddPartition(std::unique_ptr<partition::Partition> partition);

 private:
  std::unordered_map<uint32_t, std::unique_ptr<partition::Partition>>
      partitions_;
};

}  // namespace wombat::broker
