// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <vector>

namespace wombat::broker::log {

class Log {
 public:
  Log() = default;
  virtual ~Log() {}

  Log(const Log&) = delete;
  Log& operator=(const Log&) = delete;

  Log(Log&&) = default;
  Log& operator=(Log&&) = default;

  uint32_t size() const { return size_; }

  virtual void Append(const std::vector<uint8_t>& data) = 0;

  virtual std::vector<uint8_t> Lookup(uint32_t offset, uint32_t size) = 0;

 protected:
  uint32_t size_ = 0;
};

}  // namespace wombat::broker::log
