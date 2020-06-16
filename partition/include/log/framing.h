#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <vector>
#include <thread>

#include "log/log.h"

namespace wombat::log {

template<class S>
class Framing {
 public:
  Framing(Log<S> log) : log_{log} {}

  void Append(const std::vector<uint8_t>& data) {
    WriteU32(data.size());
    log_.Append(data);
  }

  bool Lookup(uint64_t offset, std::vector<uint8_t>* data, uint32_t* next) {
    uint32_t size;
    if (!ReadU32(offset, &size)) {
      return false;
    }
    // TODO(AD) log should return bool too
    *data = log_.Lookup(offset + sizeof(uint32_t), size);
    *next = offset + sizeof(uint32_t) + data->size();
    return true;
  }

 private:
  void WriteU32(uint32_t n) {
    uint32_t ordered = htonl(n);
    std::vector<uint8_t> enc {
      (uint8_t) (ordered >> 0),
      (uint8_t) (ordered >> 8),
      (uint8_t) (ordered >> 16),
      (uint8_t) (ordered >> 24)
    };
    log_.Append(enc);
  }

  bool ReadU32(uint32_t offset, uint32_t* n) {
    std::vector<uint8_t> enc = log_.Lookup(offset, sizeof(uint32_t));
    if (enc.empty()) return false;

    std::memcpy(n, enc.data(), sizeof(uint32_t));

    *n = ntohl(*n);
    return true;
  }

  Log<S> log_;
};

}  // namespace wombat::log
