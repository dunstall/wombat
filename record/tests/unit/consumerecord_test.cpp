// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "record/consumerecord.h"

namespace wombat::broker {

class ConsumeRecordTest : public ::testing::Test {};

TEST_F(ConsumeRecordTest, GetOffset) {
  const uint32_t offset = 0xfa8f23;
  const ConsumeRecord record{offset};
  EXPECT_EQ(offset, record.offset());
}

TEST_F(ConsumeRecordTest, Encode) {
  const uint32_t offset = 0xfa8f23;
  const ConsumeRecord record{offset};

  const std::vector<uint8_t> expected{0x00, 0xfa, 0x8f, 0x23};
  EXPECT_EQ(expected, record.Encode());
}

TEST_F(ConsumeRecordTest, DecodeOk) {
  const uint32_t offset = 0xfa8f23;
  const std::vector<uint8_t> enc{0x00, 0xfa, 0x8f, 0x23};

  const ConsumeRecord expected{offset};

  EXPECT_TRUE(ConsumeRecord::Decode(enc));
  EXPECT_EQ(expected, *ConsumeRecord::Decode(enc));
}

TEST_F(ConsumeRecordTest, DecodeTooSmall) {
  const std::vector<uint8_t> enc{1, 2};
  EXPECT_FALSE(ConsumeRecord::Decode(enc));
}

}  // namespace wombat::broker
