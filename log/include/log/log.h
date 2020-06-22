#pragma once

#include <filesystem>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include <glog/logging.h>
#include "logexception.h"
#include "offsets.h"

namespace wombat::log {

constexpr uint32_t OFFSET_SEGMENT_ID = 0;

template<class S>
class Log {
 public:
  Log(const std::filesystem::path& path, uint32_t segment_limit)
    : offsets_{S{OFFSET_SEGMENT_ID, path, segment_limit}},
      path_{path},
      segments_{},
      active_{1},
      segment_limit_{segment_limit},
      size_{0} {
    segments_.emplace(active_, S{active_, path, segment_limit});
    offsets_.Insert(0, active_);

    uint32_t id;
    uint32_t offset;
    offsets_.Lookup(offsets_.MaxOffset(), &id, &offset);
    size_ = offsets_.MaxOffset() + LookupSegment(id).size();
  }

  Log(const Log&) = delete;
  Log& operator=(const Log&) = delete;

  Log(Log&&) = default;
  Log& operator=(Log&&) = default;

  uint32_t size() const { return size_; }

  void Append(const std::vector<uint8_t>& data) {
    // Allow at() to throw as should never happen if the id is in offsets.
    S& segment = segments_.at(active_);
    segment.Append(data);
    if (segment.is_full()) {
      ++active_;
      segments_.emplace(active_, S{active_, path_, segment_limit_});
      offsets_.Insert(offsets_.MaxOffset() + segment.size(), active_);
      LOG(INFO) << "opening new segment: " << active_;
    }
    size_ += data.size();
  }

  std::vector<uint8_t> Lookup(uint32_t offset, uint32_t size) {
    uint32_t id;
    uint32_t starting_offset;
    if (!offsets_.Lookup(offset, &id, &starting_offset)) {
      // This should never happen a segment at offset 0 is always added.
      throw LogException("offset not found");
    }
    // TODO(AD) need to handle lookup over multi segments now
    return LookupSegment(id).Lookup(offset - starting_offset, size);
  }

  uint32_t Send(uint32_t offset, uint32_t size, int fd) {
    uint32_t written = 0;
    while (written < size) {
      uint32_t id;
      uint32_t starting_offset;
      if (!offsets_.Lookup(offset, &id, &starting_offset)) {
        // This should never happen a segment at offset 0 is always added.
        throw LogException("offset not found");
      }

      S& segment = LookupSegment(id);
      uint32_t n = segment.Send(offset - starting_offset, size - written, fd);
      // If reach EOF return the bytes already written.
      if (n == 0) {
        return written;
      }

      written += n;
      offset += n;
    }

    return written;
  }

 private:
  S& LookupSegment(uint32_t id) {
    // Lazily load the segments. TODO if too make open FDs is an issue could
    // only keep latest open (LRU cache).
    if (segments_.find(id) == segments_.end()) {
      segments_.emplace(id, S{id, path_, segment_limit_});
    }
    // Allow at() to throw as should never happen if the id is in offsets.
    return segments_.at(id);
  }

  Offsets<S> offsets_;

  std::filesystem::path path_;

  std::unordered_map<uint32_t, S> segments_;

  uint32_t active_;

  uint32_t segment_limit_;

  uint32_t size_;
};

}  // namespace wombat::log
