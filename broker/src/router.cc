// Copyright 2020 Andrew Dunstall

#include "broker/router.h"

#include <memory>
#include <filesystem>

#include "log/systemlog.h"
#include "server/event.h"

namespace wombat::broker {

bool Router::Route(const server::Event& evt) {
  if (partitions_.find(evt.message.partition_id()) != partitions_.end()) {
    partitions_.at(evt.message.partition_id())->Handle(evt);
    return true;
  }
  return false;
}

void Router::AddPartition(std::unique_ptr<Partition> partition) {
  partitions_[partition->id()] = std::move(partition);
}

}  // namespace wombat::broker
