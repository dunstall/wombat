// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>

#include <filesystem>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "log/log.h"
#include "log/offsets.h"

namespace wombat::broker::log {

constexpr uint32_t OFFSET_SEGMENT_ID = 0;

class SystemLog : public Log {
 public:
  explicit SystemLog(const std::filesystem::path& path,
                     uint32_t segment_limit = 128'000'000);

  ~SystemLog() override {}

  void Append(const std::vector<uint8_t>& data) override;

  std::vector<uint8_t> Lookup(uint32_t offset, uint32_t size) override;

 private:
  std::shared_ptr<Segment> LookupSegment(uint32_t id);

  Offsets offsets_;

  std::filesystem::path path_;

  std::unordered_map<uint32_t, std::shared_ptr<Segment>> segments_;

  uint32_t active_;

  uint32_t segment_limit_;
};

}  // namespace wombat::broker::log
