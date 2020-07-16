// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "frame/offset.h"
#include "gtest/gtest.h"

namespace wombat::broker::frame {

class OffsetTest : public ::testing::Test {};

TEST_F(OffsetTest, GetOffset) {
  const uint32_t offset = 0xfa8f23;
  const Offset request{offset};
  EXPECT_EQ(offset, request.offset());
}

TEST_F(OffsetTest, Encode) {
  const uint32_t offset = 0xfa8f23;
  const Offset request{offset};

  const std::vector<uint8_t> expected{0x00, 0xfa, 0x8f, 0x23};
  EXPECT_EQ(expected, request.Encode());
}

TEST_F(OffsetTest, DecodeOk) {
  const uint32_t offset = 0xfa8f23;
  const std::vector<uint8_t> enc{0x00, 0xfa, 0x8f, 0x23};

  const Offset expected{offset};

  EXPECT_TRUE(Offset::Decode(enc));
  EXPECT_EQ(expected, *Offset::Decode(enc));
}

TEST_F(OffsetTest, DecodeTooSmall) {
  const std::vector<uint8_t> enc{1, 2};
  EXPECT_FALSE(Offset::Decode(enc));
}

}  // namespace wombat::broker::frame
