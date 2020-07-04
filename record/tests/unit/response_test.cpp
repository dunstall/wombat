// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "record/response.h"

namespace wombat::broker::record {

class ResponseTest : public ::testing::Test {};

TEST_F(ResponseTest, Get) {
  const ResponseType type = ResponseType::kProduce;
  const std::vector<uint8_t> payload{0, 1, 2, 3};
  const Response response{type, payload};
  EXPECT_EQ(type, response.type());
  EXPECT_EQ(payload, response.payload());
}

TEST_F(ResponseTest, ExceedSizeLimit) {
  const std::vector<uint8_t> payload(513);
  EXPECT_THROW(
      Response(ResponseType::kProduce, payload),
      std::invalid_argument
  );
}

TEST_F(ResponseTest, Encode) {
  const ResponseType type = ResponseType::kConsume;
  const std::vector<uint8_t> payload{0xa, 0xb, 0xc, 0xd};
  const Response response{type, payload};

  const std::vector<uint8_t> expected{
    0x00, 0x00, 0x00, 0x01,  // Type
    0x00, 0x00, 0x00, 0x04,  // Size
    0x0a, 0x0b, 0x0c, 0x0d,  // Payload
  };
  EXPECT_EQ(expected, response.Encode());
}

TEST_F(ResponseTest, EncodeLimit) {
  const ResponseType type = ResponseType::kConsume;
  const std::vector<uint8_t> payload(0x200, 0xff);
  const Response response{type, payload};

  std::vector<uint8_t> expected{
    0x00, 0x00, 0x00, 0x01,  // Type
    0x00, 0x00, 0x02, 0x00,  // Size
  };
  expected.insert(expected.end(), payload.begin(), payload.end());

  EXPECT_EQ(expected, response.Encode());
}

TEST_F(ResponseTest, DecodeOk) {
  const std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,  // Type
    0x00, 0x00, 0x00, 0x04,  // Size
    0x0a, 0x0b, 0x0c, 0x0d,  // Payload
  };

  const ResponseType type = ResponseType::kConsume;
  const std::vector<uint8_t> payload{0xa, 0xb, 0xc, 0xd};
  const Response expected{type, payload};

  EXPECT_TRUE(Response::Decode(enc));
  EXPECT_EQ(expected, *Response::Decode(enc));
}

TEST_F(ResponseTest, DecodeHeaderTooSmall) {
  std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,
    0x00, 0x00,
  };

  EXPECT_FALSE(Response::Decode(enc));
}

TEST_F(ResponseTest, DecodePayloadTooSmall) {
  const std::vector<uint8_t> payload(0x10, 0xff);

  std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,  // Type
    0x00, 0x00, 0x02, 0x00,  // Size
  };
  enc.insert(enc.end(), payload.begin(), payload.end());

  EXPECT_FALSE(Response::Decode(enc));
}

TEST_F(ResponseTest, DecodePayloadExceedsSize) {
  const std::vector<uint8_t> payload(0x10, 0xff);

  std::vector<uint8_t> enc{
    0x00, 0x00, 0x00, 0x01,  // Type
    0x00, 0x00, 0x00, 0x05,  // Size is only 5.
  };
  enc.insert(enc.end(), payload.begin(), payload.end());

  const ResponseType type = ResponseType::kConsume;
  const std::vector<uint8_t> payload_small(0x5, 0xff);
  const Response expected{type, payload_small};

  EXPECT_TRUE(Response::Decode(enc));
  EXPECT_EQ(expected, *Response::Decode(enc));
}

TEST_F(ResponseTest, DecodeEncoded) {
  const ResponseType type = ResponseType::kConsume;
  const std::vector<uint8_t> payload{0xa, 0xb, 0xc, 0xd};
  const Response response{type, payload};

  EXPECT_TRUE(Response::Decode(response.Encode()));
  EXPECT_EQ(response, *Response::Decode(response.Encode()));
}

}  // namespace wombat::broker::record
