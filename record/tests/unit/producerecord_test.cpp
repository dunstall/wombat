#include "gtest/gtest.h"
#include "record/producerecord.h"

namespace wombat::broker {

class ProduceRecordTest : public ::testing::Test {};

TEST_F(ProduceRecordTest, GetData) {
  const std::vector<uint8_t> data{1, 2, 3, 4, 5};
  const ProduceRecord record{data};
  EXPECT_EQ(data, record.data());
}

TEST_F(ProduceRecordTest, ExceedSizeLimit) {
  const std::vector<uint8_t> data(513);
  EXPECT_THROW(ProduceRecord{data}, std::invalid_argument);
}

TEST_F(ProduceRecordTest, EncodeSmall) {
  const std::vector<uint8_t> data{1, 2, 3, 4, 5};
  const ProduceRecord record{data};

  const std::vector<uint8_t> expected{0, 0, 0, 5, 1, 2, 3, 4, 5};
  EXPECT_EQ(expected, record.Encode());
}

TEST_F(ProduceRecordTest, EncodeLimit) {
  const std::vector<uint8_t> data(0x200, 0xff);
  const ProduceRecord record{data};

  std::vector<uint8_t> expected{0, 0, 0x2, 0x0};
  expected.insert(expected.end(), data.begin(), data.end());

  EXPECT_EQ(expected, record.Encode());
}

TEST_F(ProduceRecordTest, DecodeOk) {
  std::vector<uint8_t> enc{0x0, 0x0, 0x0, 0x0f};
  const std::vector<uint8_t> data(0xf, 0xff);
  enc.insert(enc.end(), data.begin(), data.end());

  const ProduceRecord expected{data};

  EXPECT_TRUE(ProduceRecord::Decode(enc));
  EXPECT_EQ(expected, *ProduceRecord::Decode(enc));
}

TEST_F(ProduceRecordTest, DecodeTooSmall) {
  const std::vector<uint8_t> enc{1, 2};
  EXPECT_FALSE(ProduceRecord::Decode(enc));
}

TEST_F(ProduceRecordTest, DecodeExceedsLimit) {
  std::vector<uint8_t> enc{0x0, 0x0, 0x2, 0x0f};
  const std::vector<uint8_t> data(0x020f, 0xff);
  enc.insert(enc.end(), data.begin(), data.end());

  EXPECT_FALSE(ProduceRecord::Decode(enc));
}

TEST_F(ProduceRecordTest, DecodeSizesDontMatch) {
  std::vector<uint8_t> enc{0x0, 0x0, 0x0, 0xff};
  const std::vector<uint8_t> data(0x020f, 0xff);
  enc.insert(enc.end(), data.begin(), data.end());

  EXPECT_FALSE(ProduceRecord::Decode(enc));
}

}  // namespace wombat::broker
