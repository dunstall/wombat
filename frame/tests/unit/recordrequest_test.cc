// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "frame/recordrequest.h"
#include "gtest/gtest.h"

namespace wombat::broker {

class RecordRequestTest : public ::testing::Test {};

TEST_F(RecordRequestTest, GetOffset) {
  const uint32_t offset = 0xfa8f23;
  const RecordRequest request{offset};
  EXPECT_EQ(offset, request.offset());
}

TEST_F(RecordRequestTest, Encode) {
  const uint32_t offset = 0xfa8f23;
  const RecordRequest request{offset};

  const std::vector<uint8_t> expected{0x00, 0xfa, 0x8f, 0x23};
  EXPECT_EQ(expected, request.Encode());
}

TEST_F(RecordRequestTest, DecodeOk) {
  const uint32_t offset = 0xfa8f23;
  const std::vector<uint8_t> enc{0x00, 0xfa, 0x8f, 0x23};

  const RecordRequest expected{offset};

  EXPECT_TRUE(RecordRequest::Decode(enc));
  EXPECT_EQ(expected, *RecordRequest::Decode(enc));
}

TEST_F(RecordRequestTest, DecodeTooSmall) {
  const std::vector<uint8_t> enc{1, 2};
  EXPECT_FALSE(RecordRequest::Decode(enc));
}

}  // namespace wombat::broker
