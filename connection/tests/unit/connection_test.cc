// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <string>
#include <vector>

#include "connection/connection.h"
#include "connection/connectionexception.h"
#include "connection/mocksocket.h"
#include "frame/message.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace wombat::broker::connection {

class ConnectionTest : public ::testing::Test {};

TEST_F(ConnectionTest, ReceiveNoData) {
  std::unique_ptr<MockSocket> sock = std::make_unique<MockSocket>();
  EXPECT_CALL(*sock, Read(::testing::_, 0, frame::MessageHeader::kSize))
      .Times(2)
      .WillRepeatedly(::testing::Return(0));

  Connection conn{std::move(sock)};
  // Both calls should request the full header size.
  EXPECT_EQ(std::nullopt, conn.Receive());
  EXPECT_EQ(std::nullopt, conn.Receive());
}

TEST_F(ConnectionTest, ReceivePartialHeader) {
  constexpr uint32_t n = 4;
  std::unique_ptr<MockSocket> sock = std::make_unique<MockSocket>();
  EXPECT_CALL(*sock, Read(::testing::_, 0, frame::MessageHeader::kSize))
      .WillOnce(::testing::Return(n));
  // The second call should request the bytes remaining to make a full header.
  EXPECT_CALL(*sock, Read(::testing::_, n, frame::MessageHeader::kSize - n))
      .WillOnce(::testing::Return(0));

  Connection conn{std::move(sock)};
  EXPECT_EQ(std::nullopt, conn.Receive());
  EXPECT_EQ(std::nullopt, conn.Receive());
}

auto ResponseWriter(const std::vector<uint8_t>& data) {
  return [data](std::vector<uint8_t>* buf, size_t from, size_t n) -> size_t {
    buf->insert(buf->begin() + from, data.begin(), data.end());
    return data.size();
  };
}

TEST_F(ConnectionTest, ReceiveMessageWithNoPayload) {
  const frame::MessageHeader h{frame::Type::kDummy, 0, 0};
  const frame::Message m{h, {}};

  std::unique_ptr<MockSocket> sock = std::make_unique<MockSocket>();
  // Return the full message header and write this to buf.
  EXPECT_CALL(*sock, Read(::testing::_, 0, frame::MessageHeader::kSize))
      .WillOnce(ResponseWriter(h.Encode()));

  Connection conn{std::move(sock)};
  // As the message has no payload this is the full message.
  EXPECT_EQ(m, conn.Receive());
}

TEST_F(ConnectionTest, ReceiveMessageWithPayload) {
  const frame::MessageHeader h{frame::Type::kDummy, 0, 5};
  const frame::Message m{h, {1, 2, 3, 4, 5}};

  std::unique_ptr<MockSocket> sock = std::make_unique<MockSocket>();
  // Return the message header and write this to buf.
  EXPECT_CALL(*sock, Read(::testing::_, 0, frame::MessageHeader::kSize))
      .WillOnce(ResponseWriter(h.Encode()));
  // Return the message payload and write this to buf.
  EXPECT_CALL(*sock, Read(::testing::_, frame::MessageHeader::kSize,
                          m.payload().size()))
      .WillOnce(ResponseWriter(m.payload()));

  Connection conn{std::move(sock)};
  // Reads header.
  EXPECT_EQ(std::nullopt, conn.Receive());
  // Reads payload.
  EXPECT_EQ(m, conn.Receive());
}

TEST_F(ConnectionTest, ReceiveMessageOneByteAtATime) {
  const frame::MessageHeader h{frame::Type::kDummy, 0, 5};
  const frame::Message m{frame::Type::kDummy, 0, {1, 2, 3, 4, 5}};

  std::unique_ptr<MockSocket> sock = std::make_unique<MockSocket>();

  size_t from = 0;
  for (uint8_t b : h.Encode()) {
    EXPECT_CALL(*sock,
                Read(::testing::_, from, frame::MessageHeader::kSize - from))
        .WillOnce(ResponseWriter({b}));
    ++from;
  }

  size_t i = 0;
  for (uint8_t b : m.payload()) {
    EXPECT_CALL(*sock, Read(::testing::_, from, m.payload().size() - i))
        .WillOnce(ResponseWriter({b}));
    ++from;
    ++i;
  }

  Connection conn{std::move(sock)};
  for (size_t i = 0; i != m.Encode().size() - 1; ++i) {
    EXPECT_EQ(std::nullopt, conn.Receive());
  }
  EXPECT_EQ(m, conn.Receive());
}

TEST_F(ConnectionTest, ReceiveInvalidHeader) {
  // A header of all 0xff is invalid.
  const std::vector<uint8_t> data(frame::MessageHeader::kSize, 0xff);

  std::unique_ptr<MockSocket> sock = std::make_unique<MockSocket>();
  // Return the full message header and write this to buf.
  EXPECT_CALL(*sock, Read(::testing::_, 0, frame::MessageHeader::kSize))
      .WillOnce(ResponseWriter(data));

  Connection conn{std::move(sock)};
  EXPECT_THROW(conn.Receive(), ConnectionException);
}

// TODO(AD) Socket exception - use Throw

}  // namespace wombat::broker::connection
