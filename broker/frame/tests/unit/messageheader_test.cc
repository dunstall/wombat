// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "frame/messageheader.h"
#include "gtest/gtest.h"

namespace wombat::broker::frame {

class MessageHeaderTest : public ::testing::Test {};

TEST_F(MessageHeaderTest, Get) {
  const Type type = Type::kProduceRequest;
  const uint32_t partition_id = 0xaf;
  const uint32_t payload_size = 0xff;
  const MessageHeader header{type, partition_id, payload_size};
  EXPECT_EQ(type, header.type());
  EXPECT_EQ(partition_id, header.partition_id());
  EXPECT_EQ(payload_size, header.payload_size());
}

TEST_F(MessageHeaderTest, ExceedSizeLimit) {
  EXPECT_THROW(MessageHeader(Type::kProduceRequest, 0, 513),
               std::invalid_argument);
}

TEST_F(MessageHeaderTest, Encode) {
  const Type type = Type::kConsumeRequest;
  const uint32_t partition_id = 0xaabbccdd;
  const uint32_t payload_size = 0xff;
  const MessageHeader header{type, partition_id, payload_size};

  const std::vector<uint8_t> expected{
      0x00, 0x00, 0x00, 0x01,  // Type
      0xaa, 0xbb, 0xcc, 0xdd,  // Partition ID
      0x00, 0x00, 0x00, 0xff,  // Payload size
  };
  EXPECT_EQ(expected, header.Encode());
}

TEST_F(MessageHeaderTest, EncodeLimit) {
  const Type type = Type::kConsumeRequest;
  const uint32_t partition_id = 0xaabbccdd;
  const uint32_t payload_size = 0x200;
  const MessageHeader header{type, partition_id, payload_size};

  std::vector<uint8_t> expected{
      0x00, 0x00, 0x00, 0x01,  // Type
      0xaa, 0xbb, 0xcc, 0xdd,  // Partition ID
      0x00, 0x00, 0x02, 0x00,  // Payload size
  };

  EXPECT_EQ(expected, header.Encode());
}

TEST_F(MessageHeaderTest, DecodeOk) {
  const std::vector<uint8_t> enc{
      0x00, 0x00, 0x00, 0x01,  // Type
      0xaa, 0xbb, 0xcc, 0xdd,  // Partition ID
      0x00, 0x00, 0x00, 0xfa,  // Payload size
  };

  const MessageHeader expected{Type::kConsumeRequest, 0xaabbccdd, 0xfa};

  EXPECT_TRUE(MessageHeader::Decode(enc));
  EXPECT_EQ(expected, *MessageHeader::Decode(enc));
}

TEST_F(MessageHeaderTest, DecodeHeaderTooSmall) {
  std::vector<uint8_t> enc{
      0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  };

  EXPECT_FALSE(MessageHeader::Decode(enc));
}

TEST_F(MessageHeaderTest, DecodeExceedsLimit) {
  std::vector<uint8_t> enc{0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x02, 0x01};

  EXPECT_FALSE(MessageHeader::Decode(enc));
}

}  // namespace wombat::broker::frame
