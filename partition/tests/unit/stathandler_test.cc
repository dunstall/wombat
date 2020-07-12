// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "event/event.h"
#include "event/responder.h"
#include "frame/offset.h"
#include "frame/message.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "log/log.h"
#include "partition/stathandler.h"

namespace wombat::broker {

class MockLog : public log::Log {
 public:
  explicit MockLog(uint32_t size = 0) : Log{size} {}

  MOCK_METHOD(void, Append, (const std::vector<uint8_t>& data), (override));
  MOCK_METHOD(std::vector<uint8_t>, Lookup, (uint32_t offset, uint32_t size), (override));  // NOLINT
};

class MockResponder : public Responder {
 public:
  MOCK_METHOD(void, Respond, (const Event& evt), (override));
};

class FakeConnection : public Connection {
 public:
  std::optional<frame::Message> Receive() override { return std::nullopt; }

  bool Send(const frame::Message& msg) override { return false; }
};

class StatHandlerTest : public ::testing::Test {
 protected:
  const uint32_t kPartitionId = 0xffaa;
};

TEST_F(StatHandlerTest, HandleValidStatRequest) {
  const uint32_t log_size = 0xffffaaaa;

  std::shared_ptr<MockResponder> responder = std::make_shared<MockResponder>();
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>(log_size);
  StatHandler handler{kPartitionId, responder, log};

  const frame::Message msg{frame::Type::kStatRequest, kPartitionId, {}};

  std::shared_ptr<FakeConnection> conn = std::make_shared<FakeConnection>();

  const frame::Offset stat{log_size};
  const frame::Message expected_response{
      frame::Type::kStatResponse, kPartitionId, stat.Encode()
  };
  EXPECT_CALL(*responder, Respond(Event{expected_response, conn}));

  handler.Handle(Event{msg, conn});
}

TEST_F(StatHandlerTest, HandleUnrecognizedType) {
  std::shared_ptr<MockResponder> responder = std::make_shared<MockResponder>();
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  StatHandler handler{kPartitionId, responder, log};

  std::shared_ptr<FakeConnection> conn = std::make_shared<FakeConnection>();
  const frame::Message msg{
      frame::Type::kProduceRequest, kPartitionId, {0, 1, 2, 3, 4}
  };

  handler.Handle(Event{msg, conn});
}

}  // namespace wombat::broker
