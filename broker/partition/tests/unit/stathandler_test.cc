// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "connection/connection.h"
#include "connection/event.h"
#include "frame/message.h"
#include "frame/offset.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "log/log.h"
#include "log/mocklog.h"
#include "partition/stathandler.h"
#include "server/responder.h"

namespace wombat::broker::partition {

class FakeConnection : public connection::Connection {
 public:
  FakeConnection() : connection::Connection{nullptr} {}

  std::optional<frame::Message> Receive() override { return std::nullopt; }

  void Send(const frame::Message& msg) override{};
};

class StatHandlerTest : public ::testing::Test {
 public:
  const uint32_t kPartitionId = 0xffaa;
};

TEST_F(StatHandlerTest, HandleValidStatRequest) {
  const uint32_t log_size = 0xffffaaaa;

  std::shared_ptr<log::MockLog> log = std::make_shared<log::MockLog>(log_size);
  StatHandler handler{kPartitionId, log};

  const frame::Message msg{frame::Type::kStatRequest, kPartitionId, {}};

  std::shared_ptr<FakeConnection> conn = std::make_shared<FakeConnection>();

  const frame::Offset stat{log_size};
  const frame::Message expected_response{frame::Type::kStatResponse,
                                         kPartitionId, stat.Encode()};
  const connection::Event expected_event{expected_response, conn};

  EXPECT_EQ(expected_event, handler.Handle(connection::Event{msg, conn}));
}

TEST_F(StatHandlerTest, HandleUnrecognizedType) {
  std::shared_ptr<log::MockLog> log = std::make_shared<log::MockLog>();
  StatHandler handler{kPartitionId, log};

  const frame::Message msg{
      frame::Type::kProduceRequest, kPartitionId, {0, 1, 2, 3, 4}};

  EXPECT_EQ(std::nullopt, handler.Handle(connection::Event{msg, nullptr}));
}

}  // namespace wombat::broker::partition
