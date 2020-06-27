// Copyright 2020 Andrew Dunstall

#include "gtest/gtest.h"
#include "record/record.h"

namespace wombat::broker {

TEST(TestEncodeU32, Ok) {
  const uint32_t n = 0xaabbccdd;
  const std::vector<uint8_t> expected{0xaa, 0xbb, 0xcc, 0xdd};
  EXPECT_EQ(expected, EncodeU32(n));
}

TEST(TestDecodeU32, Ok) {
  const std::vector<uint8_t> enc{0xaa, 0xbb, 0xcc, 0xdd};
  const uint32_t expected = 0xaabbccdd;
  EXPECT_EQ(expected, DecodeU32(enc));
}

TEST(TestDecodeU32, InvalidTooSmall) {
  const std::vector<uint8_t> enc{0xaa, 0xbb};
  EXPECT_EQ(std::nullopt, DecodeU32(enc));
}

}  // namespace wombat::broker
