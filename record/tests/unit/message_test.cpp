// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "record/message.h"

namespace wombat::broker::record {

class MessageHeaderTest : public ::testing::Test {};

TEST_F(MessageHeaderTest, Get) {
  const MessageType type = MessageType::kProduceRequest;
  const uint32_t partition_id = 0xaf;
  const uint32_t payload_size = 0xff;
  const MessageHeader header{type, partition_id, payload_size};
  EXPECT_EQ(type, header.type());
  EXPECT_EQ(partition_id, header.partition_id());
  EXPECT_EQ(payload_size, header.payload_size());
}

TEST_F(MessageHeaderTest, ExceedSizeLimit) {
  EXPECT_THROW(
      MessageHeader(MessageType::kProduceRequest, 0, 513),
      std::invalid_argument
  );
}

TEST_F(MessageHeaderTest, ZeroSize) {
  EXPECT_THROW(
      MessageHeader(MessageType::kProduceRequest, 0, 0),
      std::invalid_argument
  );
}

TEST_F(MessageHeaderTest, Encode) {
  const MessageType type = MessageType::kConsumeRequest;
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
  const MessageType type = MessageType::kConsumeRequest;
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

  const MessageHeader expected{MessageType::kConsumeRequest, 0xaabbccdd, 0xfa};

  EXPECT_TRUE(MessageHeader::Decode(enc));
  EXPECT_EQ(expected, *MessageHeader::Decode(enc));
}

TEST_F(MessageHeaderTest, DecodeHeaderTooSmall) {
  std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,
  };

  EXPECT_FALSE(MessageHeader::Decode(enc));
}

TEST_F(MessageHeaderTest, DecodeZeroPayload) {
  std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
  };

  EXPECT_FALSE(MessageHeader::Decode(enc));
}

TEST_F(MessageHeaderTest, DecodeExceedsLimit) {
  std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x02, 0x01
  };

  EXPECT_FALSE(MessageHeader::Decode(enc));
}

class MessageTest : public ::testing::Test {};

TEST_F(MessageTest, Get) {
  const MessageType type = MessageType::kProduceRequest;
  const uint32_t partition_id = 0xaabbccdd;
  const std::vector<uint8_t> payload{0, 1, 2, 3};
  const Message request{type, partition_id, payload};
  EXPECT_EQ(type, request.type());
  EXPECT_EQ(partition_id, request.partition_id());
  EXPECT_EQ(payload, request.payload());
}

TEST_F(MessageTest, ExceedSizeLimit) {
  const uint32_t partition_id = 0xaabbccdd;
  const std::vector<uint8_t> payload(513);
  EXPECT_THROW(
      Message(MessageType::kProduceRequest, partition_id, payload),
      std::invalid_argument
  );
}

TEST_F(MessageTest, Encode) {
  const MessageType type = MessageType::kConsumeRequest;
  const uint32_t partition_id = 0xaabbccdd;
  const std::vector<uint8_t> payload{0xa, 0xb, 0xc, 0xd};
  const Message request{type, partition_id, payload};

  const std::vector<uint8_t> expected{
    0x00, 0x00, 0x00, 0x01,  // Type
    0xaa, 0xbb, 0xcc, 0xdd,  // Partition ID
    0x00, 0x00, 0x00, 0x04,  // Size
    0x0a, 0x0b, 0x0c, 0x0d,  // Payload
  };
  EXPECT_EQ(expected, request.Encode());
}

TEST_F(MessageTest, EncodeLimit) {
  const MessageType type = MessageType::kConsumeRequest;
  const uint32_t partition_id = 0xaabbccdd;
  const std::vector<uint8_t> payload(0x200, 0xff);
  const Message request{type, partition_id, payload};

  std::vector<uint8_t> expected{
    0x00, 0x00, 0x00, 0x01,  // Type
    0xaa, 0xbb, 0xcc, 0xdd,  // Partition ID
    0x00, 0x00, 0x02, 0x00,  // Size
  };
  expected.insert(expected.end(), payload.begin(), payload.end());

  EXPECT_EQ(expected, request.Encode());
}

TEST_F(MessageTest, DecodeOk) {
  const std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,  // Type
    0xaa, 0xbb, 0xcc, 0xdd,  // Partition ID
    0x00, 0x00, 0x00, 0x04,  // Size
    0x0a, 0x0b, 0x0c, 0x0d,  // Payload
  };

  const MessageType type = MessageType::kConsumeRequest;
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

  const MessageType type = MessageType::kConsumeRequest;
  const std::vector<uint8_t> payload_small(0x5, 0xff);
  const Message expected{type, 0xaabbccdd, payload_small};

  EXPECT_TRUE(Message::Decode(enc));
  EXPECT_EQ(expected, *Message::Decode(enc));
}

}  // namespace wombat::broker::record
