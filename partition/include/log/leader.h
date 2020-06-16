#pragma once

#include <vector>

#include "log/log.h"

namespace wombat::log {

struct ReplicaAddress {
  std::string ip;
  uint16_t port;
};

template<class S>
class Leader {
 public:
  // TODO(AD) Replicas should be dynamic - received from cluster membership.
  Leader(Log<S> log, const std::vector<ReplicaAddress>& replicas)
      : log_{log} {}

 private:
  Log<S> log_;
};

}  // namespace wombat::log
