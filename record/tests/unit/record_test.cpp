// Copyright 2020 Andrew Dunstall

#include "gtest/gtest.h"
#include "record/record.h"

namespace wombat::broker::record {

class RecordTest : public ::testing::Test {};

TEST_F(RecordTest, GetData) {
  const std::vector<uint8_t> data{1, 2, 3, 4, 5};
  const Record record{data};
  EXPECT_EQ(data, record.data());
}

TEST_F(RecordTest, ExceedSizeLimit) {
  const std::vector<uint8_t> data(513);
  EXPECT_THROW(Record{data}, std::invalid_argument);
}

TEST_F(RecordTest, EncodeSmall) {
  const std::vector<uint8_t> data{1, 2, 3, 4, 5};
  const Record record{data};

  const std::vector<uint8_t> expected{0, 0, 0, 5, 1, 2, 3, 4, 5};
  EXPECT_EQ(expected, record.Encode());
}

TEST_F(RecordTest, EncodeLimit) {
  const std::vector<uint8_t> data(0x200, 0xff);
  const Record record{data};

  std::vector<uint8_t> expected{0, 0, 0x2, 0x0};
  expected.insert(expected.end(), data.begin(), data.end());

  EXPECT_EQ(expected, record.Encode());
}

TEST_F(RecordTest, DecodeOk) {
  std::vector<uint8_t> enc{0x0, 0x0, 0x0, 0x0f};
  const std::vector<uint8_t> data(0xf, 0xff);
  enc.insert(enc.end(), data.begin(), data.end());

  const Record expected{data};

  EXPECT_TRUE(Record::Decode(enc));
  EXPECT_EQ(expected, *Record::Decode(enc));
}

TEST_F(RecordTest, DecodeTooSmall) {
  const std::vector<uint8_t> enc{1, 2};
  EXPECT_FALSE(Record::Decode(enc));
}

TEST_F(RecordTest, DecodeExceedsLimit) {
  std::vector<uint8_t> enc{0x0, 0x0, 0x2, 0x0f};
  const std::vector<uint8_t> data(0x020f, 0xff);
  enc.insert(enc.end(), data.begin(), data.end());

  EXPECT_FALSE(Record::Decode(enc));
}

TEST_F(RecordTest, DecodeSizesDontMatch) {
  std::vector<uint8_t> enc{0x0, 0x0, 0x1, 0xff};
  const std::vector<uint8_t> data(0x000f, 0xff);
  enc.insert(enc.end(), data.begin(), data.end());

  EXPECT_FALSE(Record::Decode(enc));
}

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

}  // namespace wombat::broker::record
