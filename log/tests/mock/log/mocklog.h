// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <vector>

#include "gmock/gmock.h"
#include "log/log.h"

namespace wombat::broker::log {

class MockLog : public log::Log {
 public:
  explicit MockLog(uint32_t size = 0) : Log{size} {}

  MOCK_METHOD(void, Append, (const std::vector<uint8_t>& data), (override));

  MOCK_METHOD(std::vector<uint8_t>, Lookup, (uint32_t offset, uint32_t size),
              (override));
};

}  // namespace wombat::broker::log
