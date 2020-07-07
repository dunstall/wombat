// Copyright 2020 Andrew Dunstall

#include "broker/router.h"

#include <memory>
#include <filesystem>

#include "log/systemlog.h"
#include "server/event.h"

namespace wombat::broker {

// TODO(AD) 1: add partition ID to message
// TODO(AD) 2: implement router with message partition ID

void Router::Route(const server::Event& request) {
  // TODO(AD)
}

void Router::AddPartition(std::unique_ptr<Partition> partition) {
  // TODO(AD)
}

}  // namespace wombat::broker
