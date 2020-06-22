#pragma once

#include <arpa/inet.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <vector>

namespace wombat::log {

template<class S>
class Offsets {
 public:
  Offsets(S segment) : offsets_{}, segment_{std::move(segment)} {
    uint32_t offset = 0;
    while (LoadOffset(offset)) {
      offset += 8;
    }
  }

  bool Lookup(uint32_t offset, uint32_t* id, uint32_t* start) {
    for (auto it = offsets_.rbegin(); it != offsets_.rend(); ++it) {
      if (it->first <= offset) {
        *start = it->first;
        *id = it->second;
        return true;
      }
    }
    return false;
  }

  uint32_t MaxOffset() {
    if (!offsets_.empty()) {
      return offsets_.rbegin()->first;
    }
    return 0;
  }

  void Insert(uint32_t offset, uint32_t id) {
    WriteU32(offset);
    WriteU32(id);
    offsets_.emplace(offset, id);
  }

 private:
  bool LoadOffset(uint32_t offset) {
    uint32_t loaded_offset;
    if (!ReadU32(offset, &loaded_offset)) return false;
    uint32_t loaded_id;
    if (!ReadU32(offset + 4, &loaded_id)) return false;
    offsets_.emplace(loaded_offset, loaded_id);

    return true;
  }

  void WriteU32(uint32_t n) {
    uint32_t ordered = htonl(n);
    std::vector<uint8_t> enc {
      (uint8_t) (ordered >> 0),
      (uint8_t) (ordered >> 8),
      (uint8_t) (ordered >> 16),
      (uint8_t) (ordered >> 24)
    };
    segment_.Append(enc);
  }

  bool ReadU32(uint32_t offset, uint32_t* n) {
    std::vector<uint8_t> enc = segment_.Lookup(offset, 4);
    if (enc.empty()) return false;

    std::memcpy(n, enc.data(), 4);

    *n = ntohl(*n);
    return true;
  }

  std::map<uint32_t, uint32_t> offsets_;

  S segment_;
};

}  // namespace wombat::log
