// Copyright 2020 Andrew Dunstall

#include "frame/record.h"
#include "gtest/gtest.h"

namespace wombat::broker::frame {

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

}  // namespace wombat::broker::frame
