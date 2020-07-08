// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <filesystem>
#include <memory>

#include "log/inmemorysegment.h"
#include "log/inmemorysegmentfactory.h"

namespace wombat::broker::log {

InMemorySegmentFactory::InMemorySegmentFactory(const std::filesystem::path& dir,
                                           uint32_t limit)
    : dir_{dir}, limit_{limit} {}

std::shared_ptr<Segment> InMemorySegmentFactory::Open(uint32_t id) const {
  return std::make_shared<InMemorySegment>(id, dir_, limit_);
}

}  // namespace wombat::broker::log
