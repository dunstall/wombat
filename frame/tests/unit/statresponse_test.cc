// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "frame/statresponse.h"
#include "gtest/gtest.h"

namespace wombat::broker::record {

class StatResponseTest : public ::testing::Test {};

TEST_F(StatResponseTest, GetOffset) {
  const uint32_t size = 0xfa8f23;
  const StatResponse response{size};
  EXPECT_EQ(size, response.size());
}

TEST_F(StatResponseTest, Encode) {
  const uint32_t size = 0xfa8f23;
  const StatResponse response{size};

  const std::vector<uint8_t> expected{0x00, 0xfa, 0x8f, 0x23};
  EXPECT_EQ(expected, response.Encode());
}

TEST_F(StatResponseTest, DecodeOk) {
  const uint32_t size = 0xfa8f23;
  const std::vector<uint8_t> enc{0x00, 0xfa, 0x8f, 0x23};

  const StatResponse expected{size};

  EXPECT_TRUE(StatResponse::Decode(enc));
  EXPECT_EQ(expected, *StatResponse::Decode(enc));
}

TEST_F(StatResponseTest, DecodeTooSmall) {
  const std::vector<uint8_t> enc{1, 2};
  EXPECT_FALSE(StatResponse::Decode(enc));
}

}  // namespace wombat::broker::record
