// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include "log/log.h"

namespace wombat::broker::log {

class SystemLog : public Log {
 public:
  explicit SystemLog(const std::filesystem::path& path,
                     uint32_t segment_limit = 128'000'000) {}

  ~SystemLog() override {}

  void Append(const std::vector<uint8_t>& data) override {}

  std::vector<uint8_t> Lookup(uint32_t offset, uint32_t size) override {
    return {};
  }

  uint32_t Send(uint32_t offset, uint32_t size, int fd) override {
    return 0;
  }
};

}  // namespace wombat::broker::log
