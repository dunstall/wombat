// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "record/request.h"

namespace wombat::broker::record {

class RequestHeaderTest : public ::testing::Test {};

TEST_F(RequestHeaderTest, Get) {
  const RequestType type = RequestType::kProduceRecord;
  const uint32_t payload_size = 0xff;
  const RequestHeader header{type, payload_size};
  EXPECT_EQ(type, header.type());
  EXPECT_EQ(payload_size, header.payload_size());
}

TEST_F(RequestHeaderTest, ExceedSizeLimit) {
  EXPECT_THROW(
      RequestHeader(RequestType::kProduceRecord, 513),
      std::invalid_argument
  );
}

TEST_F(RequestHeaderTest, Encode) {
  const RequestType type = RequestType::kConsumeRecord;
  const uint32_t payload_size = 0xff;
  const RequestHeader header{type, payload_size};

  const std::vector<uint8_t> expected{
    0x00, 0x00, 0x00, 0x01,  // Type
    0x00, 0x00, 0x00, 0xff,  // Payload size
  };
  EXPECT_EQ(expected, header.Encode());
}

TEST_F(RequestHeaderTest, EncodeLimit) {
  const RequestType type = RequestType::kConsumeRecord;
  const uint32_t payload_size = 0x200;
  const RequestHeader header{type, payload_size};

  std::vector<uint8_t> expected{
    0x00, 0x00, 0x00, 0x01,  // Type
    0x00, 0x00, 0x02, 0x00,  // Payload size
  };

  EXPECT_EQ(expected, header.Encode());
}

TEST_F(RequestHeaderTest, DecodeOk) {
  const std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,  // Type
    0x00, 0x00, 0x00, 0xfa,  // Payload size
  };

  const RequestHeader expected{RequestType::kConsumeRecord, 0xfa};

  EXPECT_TRUE(RequestHeader::Decode(enc));
  EXPECT_EQ(expected, *RequestHeader::Decode(enc));
}

TEST_F(RequestHeaderTest, DecodeHeaderTooSmall) {
  std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,
    0x00, 0x00,
  };

  EXPECT_FALSE(RequestHeader::Decode(enc));
}

class RequestTest : public ::testing::Test {};

TEST_F(RequestTest, Get) {
  const RequestType type = RequestType::kProduceRecord;
  const std::vector<uint8_t> payload{0, 1, 2, 3};
  const Request request{type, payload};
  EXPECT_EQ(type, request.type());
  EXPECT_EQ(payload, request.payload());
}

TEST_F(RequestTest, ExceedSizeLimit) {
  const std::vector<uint8_t> payload(513);
  EXPECT_THROW(
      Request(RequestType::kProduceRecord, payload),
      std::invalid_argument
  );
}

TEST_F(RequestTest, Encode) {
  const RequestType type = RequestType::kConsumeRecord;
  const std::vector<uint8_t> payload{0xa, 0xb, 0xc, 0xd};
  const Request request{type, payload};

  const std::vector<uint8_t> expected{
    0x00, 0x00, 0x00, 0x01,  // Type
    0x00, 0x00, 0x00, 0x04,  // Size
    0x0a, 0x0b, 0x0c, 0x0d,  // Payload
  };
  EXPECT_EQ(expected, request.Encode());
}

TEST_F(RequestTest, EncodeLimit) {
  const RequestType type = RequestType::kConsumeRecord;
  const std::vector<uint8_t> payload(0x200, 0xff);
  const Request request{type, payload};

  std::vector<uint8_t> expected{
    0x00, 0x00, 0x00, 0x01,  // Type
    0x00, 0x00, 0x02, 0x00,  // Size
  };
  expected.insert(expected.end(), payload.begin(), payload.end());

  EXPECT_EQ(expected, request.Encode());
}

TEST_F(RequestTest, DecodeOk) {
  const std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,  // Type
    0x00, 0x00, 0x00, 0x04,  // Size
    0x0a, 0x0b, 0x0c, 0x0d,  // Payload
  };

  const RequestType type = RequestType::kConsumeRecord;
  const std::vector<uint8_t> payload{0xa, 0xb, 0xc, 0xd};
  const Request expected{type, payload};

  EXPECT_TRUE(Request::Decode(enc));
  EXPECT_EQ(expected, *Request::Decode(enc));
}

TEST_F(RequestTest, DecodeHeaderTooSmall) {
  std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,
    0x00, 0x00,
  };

  EXPECT_FALSE(Request::Decode(enc));
}

TEST_F(RequestTest, DecodePayloadTooSmall) {
  const std::vector<uint8_t> payload(0x10, 0xff);

  std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,  // Type
    0x00, 0x00, 0x02, 0x00,  // Size
  };
  enc.insert(enc.end(), payload.begin(), payload.end());

  EXPECT_FALSE(Request::Decode(enc));
}

TEST_F(RequestTest, DecodePayloadExceedsSize) {
  const std::vector<uint8_t> payload(0x10, 0xff);

  std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,  // Type
    0x00, 0x00, 0x00, 0x05,  // Size is only 5.
  };
  enc.insert(enc.end(), payload.begin(), payload.end());

  const RequestType type = RequestType::kConsumeRecord;
  const std::vector<uint8_t> payload_small(0x5, 0xff);
  const Request expected{type, payload_small};

  EXPECT_TRUE(Request::Decode(enc));
  EXPECT_EQ(expected, *Request::Decode(enc));
}

TEST_F(RequestTest, DecodeEncoded) {
  const RequestType type = RequestType::kConsumeRecord;
  const std::vector<uint8_t> payload{0xa, 0xb, 0xc, 0xd};
  const Request request{type, payload};

  EXPECT_TRUE(Request::Decode(request.Encode()));
  EXPECT_EQ(request, *Request::Decode(request.Encode()));
}

}  // namespace wombat::broker::record
