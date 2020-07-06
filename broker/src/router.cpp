// Copyright 2020 Andrew Dunstall

#include "broker/router.h"

#include <filesystem>

#include "log/systemlog.h"

namespace wombat::broker {

// TODO(AD) Dont need request/response events - just have Event{conn, message}
void Router::Route(const server::Event& request) {
  // TODO(AD)
}

void Router::AddPartition(std::unique_ptr<Partition> partition) {
  // TODO(AD)
}

}  // namespace wombat::broker
