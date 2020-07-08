// Copyright 2020 Andrew Dunstall

#include "log/logimpl.h"

#include <filesystem>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "glog/logging.h"
#include "log/logexception.h"
#include "log/offsets.h"
#include "log/segmentfactory.h"

namespace wombat::broker::log {

LogImpl::LogImpl(std::unique_ptr<SegmentFactory> segment_factory)
  : offsets_{segment_factory->Open(OFFSET_SEGMENT_ID)},
    segments_{},
    active_{1},
    size_{0},
    segment_factory_{std::move(segment_factory)} {
  segments_.emplace(active_, segment_factory->Open(active_));
  offsets_.Insert(0, active_);

  uint32_t id;
  uint32_t offset;
  offsets_.Lookup(offsets_.MaxOffset(), &id, &offset);
  size_ = offsets_.MaxOffset() + LookupSegment(id)->size();
}

void LogImpl::Append(const std::vector<uint8_t>& data) {
  // Allow at() to throw as should never happen if the id is in offsets.
  std::shared_ptr<Segment> segment = segments_.at(active_);
  segment->Append(data);
  if (segment->is_full()) {
    ++active_;
    segments_.emplace(active_, segment_factory_->Open(active_));
    offsets_.Insert(offsets_.MaxOffset() + segment->size(), active_);
    LOG(INFO) << "opening new segment: " << active_;
  }
  size_ += data.size();
}

std::vector<uint8_t> LogImpl::Lookup(uint32_t offset, uint32_t size) {
  uint32_t id;
  uint32_t starting_offset;
  if (!offsets_.Lookup(offset, &id, &starting_offset)) {
    // This should never happen a segment at offset 0 is always added.
    throw LogException("offset not found");
  }
  // TODO(AD) need to handle lookup over multi segments now
  return LookupSegment(id)->Lookup(offset - starting_offset, size);
}

uint32_t LogImpl::Send(uint32_t offset, uint32_t size, int fd) {
  uint32_t written = 0;
  while (written < size) {
    uint32_t id;
    uint32_t starting_offset;
    if (!offsets_.Lookup(offset, &id, &starting_offset)) {
      // This should never happen a segment at offset 0 is always added.
      throw LogException("offset not found");
    }

    std::shared_ptr<Segment> segment = LookupSegment(id);
    uint32_t n = segment->Send(offset - starting_offset, size - written, fd);
    // If reach EOF return the bytes already written.
    if (n == 0) {
      return written;
    }

    written += n;
    offset += n;
  }

  return written;
}

std::shared_ptr<Segment> LogImpl::LookupSegment(uint32_t id) {
  // Lazily load the segments. TODO if too make open FDs is an issue could
  // only keep latest open (LRU cache).
  if (segments_.find(id) == segments_.end()) {
    segments_.emplace(id, segment_factory_->Open(id));
  }
  // Allow at() to throw as should never happen if the id is in offsets.
  return segments_.at(id);
}

}  // namespace wombat::broker::log
