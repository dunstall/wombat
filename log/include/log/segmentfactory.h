// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <memory>

#include "log/segment.h"

namespace wombat::broker::log {

class SegmentFactory {
 public:
  virtual ~SegmentFactory() {}

  virtual std::shared_ptr<Segment> Open(uint32_t id) const = 0;
};

}  // namespace wombat::broker::log
