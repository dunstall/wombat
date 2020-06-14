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

  // TODO
  // Log(const Log&) = delete;
  // Log& operator=(const Log&) = delete;

  // Log(Log&&) = default;
  // Log& operator=(Log&&) = default;

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
    return LookupSegment(id).Lookup(offset - starting_offset, size);
  }

  uint64_t Send(uint64_t offset, uint64_t size, int fd) {
    uint64_t written = 0;
    while (written < size) {
      uint32_t id;
      uint32_t starting_offset;
      if (!offsets_.Lookup(offset, &id, &starting_offset)) {
        // This should never happen a segment at offset 0 is always added.
        throw LogException("offset not found");
      }

      S segment = LookupSegment(id);
      uint64_t n = segment.Send(offset - starting_offset, size - written, fd);
      // If reach EOF return the bytes already written.
      if (n == 0) {
        return written;
      }

      written += n;
      offset += n;
    }

    return written;
  }

  uint64_t Recv(uint64_t size, int fd) {
    // TODO(AD)
    // Must ensure the segments on each replica are identical - this is so
    // Lookup() never reaches EOF incorrectly (as record split among segments)
    // and so segments can be transfered in parallel later.
    return 0;
  }

 private:
  S LookupSegment(uint64_t id) {
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

  std::unordered_map<uint64_t, S> segments_;

  uint64_t active_;

  uint64_t segment_limit_;
};

}  // namespace wombat::log
