// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "record/consumerequest.h"

namespace wombat::broker::record {

class ConsumeRequestTest : public ::testing::Test {};

TEST_F(ConsumeRequestTest, GetOffset) {
  const uint32_t offset = 0xfa8f23;
  const ConsumeRequest record{offset};
  EXPECT_EQ(offset, record.offset());
}

TEST_F(ConsumeRequestTest, Encode) {
  const uint32_t offset = 0xfa8f23;
  const ConsumeRequest record{offset};

  const std::vector<uint8_t> expected{0x00, 0xfa, 0x8f, 0x23};
  EXPECT_EQ(expected, record.Encode());
}

TEST_F(ConsumeRequestTest, DecodeOk) {
  const uint32_t offset = 0xfa8f23;
  const std::vector<uint8_t> enc{0x00, 0xfa, 0x8f, 0x23};

  const ConsumeRequest expected{offset};

  EXPECT_TRUE(ConsumeRequest::Decode(enc));
  EXPECT_EQ(expected, *ConsumeRequest::Decode(enc));
}

TEST_F(ConsumeRequestTest, DecodeTooSmall) {
  const std::vector<uint8_t> enc{1, 2};
  EXPECT_FALSE(ConsumeRequest::Decode(enc));
}

}  // namespace wombat::broker::record
