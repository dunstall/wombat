// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <vector>

namespace wombat::broker::log {

class Log {
 public:
  virtual ~Log() {}

  uint32_t size() const { return size_; }

  virtual void Append(const std::vector<uint8_t>& data) = 0;

  virtual std::vector<uint8_t> Lookup(uint32_t offset, uint32_t size) = 0;

  virtual uint32_t Send(uint32_t offset, uint32_t size, int fd) = 0;

 protected:
  uint32_t size_;
};

}  // namespace wombat::broker::log
