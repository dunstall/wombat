// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <string>
#include <vector>

#include "connection/connection.h"
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

TEST_F(ConnectionTest, ReceiveMessageWithNoPayload) {
  const frame::MessageHeader h{frame::Type::kDummy, 0, 0};
  const frame::Message m{h, {}};

  std::unique_ptr<MockSocket> sock = std::make_unique<MockSocket>();
  // Return the full message header and write this to buf.
  EXPECT_CALL(*sock, Read(::testing::_, 0, frame::MessageHeader::kSize)).WillOnce(
      [&](std::vector<uint8_t>* buf, size_t from, size_t n) -> size_t {
          const auto data = h.Encode();
          buf->insert(buf->begin(), data.begin(), data.end());
          return frame::MessageHeader::kSize;
      }
  );

  Connection conn{std::move(sock)};
  // As the message has no payload this is the full message.
  EXPECT_EQ(m, conn.Receive());
}

TEST_F(ConnectionTest, ReceiveMessageWithPayload) {
  const frame::MessageHeader h{frame::Type::kDummy, 0, 5};
  const frame::Message m{h, {1, 2, 3, 4, 5}};

  std::unique_ptr<MockSocket> sock = std::make_unique<MockSocket>();
  // Return the message header and write this to buf.
  EXPECT_CALL(*sock, Read(::testing::_, 0, frame::MessageHeader::kSize)).WillOnce(
      [&](std::vector<uint8_t>* buf, size_t from, size_t n) -> size_t {
          const auto data = h.Encode();
          buf->insert(buf->begin(), data.begin(), data.end());
          return frame::MessageHeader::kSize;
      }
  );
  // Return the message payload and write this to buf.
  EXPECT_CALL(*sock, Read(::testing::_, frame::MessageHeader::kSize, m.payload().size())).WillOnce(
      [&](std::vector<uint8_t>* buf, size_t from, size_t n) -> size_t {
          const auto data = m.payload();
          buf->insert(
              buf->begin() + from,
              data.begin(),
              data.end()
          );
          return m.payload().size();
      }
  );

  Connection conn{std::move(sock)};
  // Reads header.
  EXPECT_EQ(std::nullopt, conn.Receive());
  // Reads payload.
  EXPECT_EQ(m, conn.Receive());
}

// TODO(AD) Receive message in increments (1 bytes)

// TODO(AD) Invalid header

// TODO(AD) Invalid message

// TODO(AD) Socket exception

}  // namespace wombat::broker::connection
