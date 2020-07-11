// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "frame/message.h"
#include "gtest/gtest.h"

namespace wombat::broker {

class MessageTest : public ::testing::Test {};

TEST_F(MessageTest, Get) {
  const Type type = Type::kProduceRequest;
  const uint32_t partition_id = 0xaabbccdd;
  const std::vector<uint8_t> payload{0, 1, 2, 3};
  const Message message{type, partition_id, payload};
  EXPECT_EQ(type, message.type());
  EXPECT_EQ(partition_id, message.partition_id());
  EXPECT_EQ(payload, message.payload());
}

TEST_F(MessageTest, ExceedSizeLimit) {
  const uint32_t partition_id = 0xaabbccdd;
  const std::vector<uint8_t> payload(513);
  EXPECT_THROW(
      Message(Type::kProduceRequest, partition_id, payload),
      std::invalid_argument
  );
}

TEST_F(MessageTest, Encode) {
  const Type type = Type::kConsumeRequest;
  const uint32_t partition_id = 0xaabbccdd;
  const std::vector<uint8_t> payload{0xa, 0xb, 0xc, 0xd};
  const Message message{type, partition_id, payload};

  const std::vector<uint8_t> expected{
    0x00, 0x00, 0x00, 0x01,  // Type
    0xaa, 0xbb, 0xcc, 0xdd,  // Partition ID
    0x00, 0x00, 0x00, 0x04,  // Size
    0x0a, 0x0b, 0x0c, 0x0d,  // Payload
  };
  EXPECT_EQ(expected, message.Encode());
}

TEST_F(MessageTest, EncodeLimit) {
  const Type type = Type::kConsumeRequest;
  const uint32_t partition_id = 0xaabbccdd;
  const std::vector<uint8_t> payload(0x200, 0xff);
  const Message message{type, partition_id, payload};

  std::vector<uint8_t> expected{
    0x00, 0x00, 0x00, 0x01,  // Type
    0xaa, 0xbb, 0xcc, 0xdd,  // Partition ID
    0x00, 0x00, 0x02, 0x00,  // Size
  };
  expected.insert(expected.end(), payload.begin(), payload.end());

  EXPECT_EQ(expected, message.Encode());
}

TEST_F(MessageTest, DecodeOk) {
  const std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,  // Type
    0xaa, 0xbb, 0xcc, 0xdd,  // Partition ID
    0x00, 0x00, 0x00, 0x04,  // Size
    0x0a, 0x0b, 0x0c, 0x0d,  // Payload
  };

  const Type type = Type::kConsumeRequest;
  const std::vector<uint8_t> payload{0xa, 0xb, 0xc, 0xd};
  const Message expected{type, 0xaabbccdd, payload};

  EXPECT_TRUE(Message::Decode(enc));
  EXPECT_EQ(expected, *Message::Decode(enc));
}

TEST_F(MessageTest, DecodeHeaderTooSmall) {
  std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,
    0x00, 0x00,
  };

  EXPECT_FALSE(Message::Decode(enc));
}

TEST_F(MessageTest, DecodePayloadTooSmall) {
  const std::vector<uint8_t> payload(0x10, 0xff);

  std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,  // Type
    0xaa, 0xbb, 0xcc, 0xdd,  // Partition ID
    0x00, 0x00, 0x02, 0x00,  // Size
  };
  enc.insert(enc.end(), payload.begin(), payload.end());

  EXPECT_FALSE(Message::Decode(enc));
}

TEST_F(MessageTest, DecodePayloadExceedsSize) {
  const std::vector<uint8_t> payload(0x10, 0xff);

  std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,  // Type
    0xaa, 0xbb, 0xcc, 0xdd,  // Partition ID
    0x00, 0x00, 0x00, 0x05,  // Size is only 5.
  };
  enc.insert(enc.end(), payload.begin(), payload.end());

  const Type type = Type::kConsumeRequest;
  const std::vector<uint8_t> payload_small(0x5, 0xff);
  const Message expected{type, 0xaabbccdd, payload_small};

  EXPECT_TRUE(Message::Decode(enc));
  EXPECT_EQ(expected, *Message::Decode(enc));
}

}  // namespace wombat::broker
