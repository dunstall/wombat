// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>

#include "gmock/gmock.h"
#include "partition/partition.h"

namespace wombat::broker::partition {

class MockPartition : public partition::Partition {
 public:
  explicit MockPartition(uint32_t id) : partition::Partition{id, nullptr} {}

  MOCK_METHOD(void, Handle, (const Event& evt), (override));

  void Process() override {}
};

}  // namespace wombat::broker::partition
