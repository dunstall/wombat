// Copyright 2020 Andrew Dunstall

#include "log/systemlog.h"

#include <cstdint>
#include <memory>

#include "glog/logging.h"
#include "log/logexception.h"
#include "log/offsets.h"
#include "log/systemsegment.h"

namespace wombat::broker::log {

SystemLog::SystemLog(const std::filesystem::path& path,
                     uint32_t segment_limit)
  : offsets_{
      std::make_shared<SystemSegment>(OFFSET_SEGMENT_ID, path, segment_limit)
    },
    path_{path},
    segments_{},
    active_{1},
    segment_limit_{segment_limit} {
  segments_.emplace(
      active_, std::make_shared<SystemSegment>(active_, path, segment_limit)
  );
  offsets_.Insert(0, active_);

  uint32_t id;
  uint32_t offset;
  offsets_.Lookup(offsets_.MaxOffset(), &id, &offset);
  size_ = offsets_.MaxOffset() + LookupSegment(id)->size();
}

void SystemLog::Append(const std::vector<uint8_t>& data) {
  // Allow at() to throw as should never happen if the id is in offsets.
  std::shared_ptr<Segment> segment = segments_.at(active_);
  segment->Append(data);
  if (segment->is_full()) {
    ++active_;
    segments_.emplace(
        active_,
        std::make_shared<SystemSegment>(active_, path_, segment_limit_)
    );
    offsets_.Insert(offsets_.MaxOffset() + segment->size(), active_);
    LOG(INFO) << "opening new segment: " << active_;
  }
  size_ += data.size();
}

std::vector<uint8_t> SystemLog::Lookup(uint32_t offset, uint32_t size) {
  uint32_t id;
  uint32_t starting_offset;
  if (!offsets_.Lookup(offset, &id, &starting_offset)) {
    // This should never happen a segment at offset 0 is always added.
    throw LogException("offset not found");
  }
  return LookupSegment(id)->Lookup(offset - starting_offset, size);
}

std::shared_ptr<Segment> SystemLog::LookupSegment(uint32_t id) {
  // Lazily load the segments. TODO(AD) if too make open FDs is an issue could
  // only keep latest open (LRU cache).
  if (segments_.find(id) == segments_.end()) {
    segments_.emplace(
        id, std::make_shared<SystemSegment>(id, path_, segment_limit_)
    );
  }
  // Allow at() to throw as should never happen if the id is in offsets.
  return segments_.at(id);
}

}  // namespace wombat::broker::log
