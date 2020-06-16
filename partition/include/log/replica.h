#pragma once

#include "log/log.h"

namespace wombat::log {

template<class S>
class Replica {
 public:
  Replica(Log<S> log) : log_{log} {}

 private:
  Log<S> log_;
};

}  // namespace wombat::log
