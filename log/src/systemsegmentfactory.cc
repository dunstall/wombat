// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <filesystem>
#include <memory>

#include "log/systemsegment.h"
#include "log/systemsegmentfactory.h"

namespace wombat::broker::log {

SystemSegmentFactory::SystemSegmentFactory(const std::filesystem::path& dir,
                                           uint32_t limit)
    : dir_{dir}, limit_{limit} {}

std::shared_ptr<Segment> SystemSegmentFactory::Open(uint32_t id) const {
  return std::make_shared<SystemSegment>(id, dir_, limit_);
}

}  // namespace wombat::broker::log
