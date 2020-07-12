// Copyright 2020 Andrew Dunstall

#include "broker/router.h"

#include <memory>
#include <filesystem>

#include "event/event.h"
#include "log/systemlog.h"
#include "partition/partition.h"

namespace wombat::broker {

bool Router::Route(const Event& evt) {
  if (partitions_.find(evt.message.partition_id()) != partitions_.end()) {
    partitions_.at(evt.message.partition_id())->Handle(evt);
    return true;
  }
  return false;
}

void Router::AddPartition(std::unique_ptr<partition::Partition> partition) {
  partitions_[partition->id()] = std::move(partition);
}

}  // namespace wombat::broker
