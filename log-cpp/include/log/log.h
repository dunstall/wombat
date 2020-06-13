#pragma once

#include <filesystem>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

#include "logexception.h"
#include "offsets.h"

namespace wombat::log {

constexpr uint64_t OFFSET_SEGMENT_ID = 0;

template<class S>
class Log {
 public:
  Log(const std::filesystem::path& path, size_t segment_limit)
    : offsets_{S{OFFSET_SEGMENT_ID, path, segment_limit}},
      path_{path},
      segments_{},
      active_{1},
      segment_limit_{segment_limit} {
    segments_.emplace(active_, S{active_, path, segment_limit});
    offsets_.Insert(0, active_);
  }

  void Append(const std::vector<uint8_t>& data) {
    // Allow at() to throw as should never happen if the id is in offsets.
    S segment = segments_.at(active_);
    segment.Append(data);
    if (segment.is_full()) {
      ++active_;
      segments_.emplace(active_, S{active_, path_, segment_limit_});
      offsets_.Insert(offsets_.MaxOffset() + segment.size(), active_);
    }
  }

  std::vector<uint8_t> Lookup(uint64_t offset, uint64_t size) {
    uint32_t id;
    uint32_t starting_offset;
    if (!offsets_.Lookup(offset, &id, &starting_offset)) {
      // This should never happen a segment at offset 0 is always added.
      throw LogException("offset not found");
    }

    // Lazily load the segments. TODO if too make open FDs is an issue could
    // only keep latest open (LRU cache).
    if (segments_.find(id) == segments_.end()) {
      segments_.emplace(id, S{id, path_, segment_limit_});
    }

    // Allow at() to throw as should never happen if the id is in offsets.
    return segments_.at(id).Lookup(offset - starting_offset, size);
  }

 private:
  Offsets<S> offsets_;

  std::filesystem::path path_;

  std::unordered_map<uint64_t, S> segments_;

  uint64_t active_;

  uint64_t segment_limit_;
};

}  // namespace wombat::log
