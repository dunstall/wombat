// Copyright 2020 Andrew Dunstall

#include "broker/router.h"

#include <memory>
#include <filesystem>

#include "log/systemlog.h"
#include "server/event.h"

namespace wombat::broker {

void Router::Route(const server::Event& request) {
  // TODO(AD)
}

void Router::AddPartition(std::unique_ptr<Partition> partition) {
  // TODO(AD)
}

}  // namespace wombat::broker
