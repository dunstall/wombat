// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "event/event.h"
#include "event/responder.h"
#include "frame/message.h"
#include "frame/offset.h"
#include "frame/record.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "log/log.h"
#include "partition/consumehandler.h"

namespace wombat::broker {

class MockLog : public log::Log {
 public:
  MOCK_METHOD(void, Append, (const std::vector<uint8_t>& data), (override));
  MOCK_METHOD(std::vector<uint8_t>, Lookup, (uint32_t offset, uint32_t size), (override));  // NOLINT
};

class MockResponder : public Responder {
 public:
  MOCK_METHOD(void, Respond, (const Event& evt), (override));
};

class FakeConnection : public Connection {
 public:
  std::optional<Message> Receive() override { return std::nullopt; }

  bool Send(const Message& msg) override { return false; }
};

class ConsumeHandlerTest : public ::testing::Test {};

TEST_F(ConsumeHandlerTest, HandleValidConsumeRequest) {
  std::shared_ptr<MockResponder> responder = std::make_shared<MockResponder>();
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  ConsumeHandler handler{responder, log};

  const std::vector<uint8_t> payload{1, 2, 3, 4, 5};
  const Record record{payload};
  const std::vector<uint8_t> encoded = record.Encode();
  const std::vector<uint8_t> encoded_size(
      encoded.begin(), encoded.begin() + 4
  );

  const uint32_t offset = 0xff;
  EXPECT_CALL(*log, Lookup(offset, sizeof(uint32_t)))
      .WillOnce(::testing::Return(encoded_size));
  EXPECT_CALL(*log, Lookup(offset, encoded.size()))
      .WillOnce(::testing::Return(encoded));

  const Offset rr{offset};
  const Message msg{
      Type::kConsumeRequest, 0, rr.Encode()
  };

  std::shared_ptr<FakeConnection> conn = std::make_shared<FakeConnection>();

  const Message expected_response{
      Type::kConsumeResponse, 0, record.Encode()
  };
  EXPECT_CALL(*responder, Respond(Event{expected_response, conn}));

  handler.Handle(Event{msg, conn});
}

TEST_F(ConsumeHandlerTest, HandleOffsetExceedsLogSize) {
  std::shared_ptr<MockResponder> responder = std::make_shared<MockResponder>();
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  ConsumeHandler handler{responder, log};

  const uint32_t offset = 0xff;
  EXPECT_CALL(*log, Lookup(offset, sizeof(uint32_t)))
      .WillOnce(::testing::Return(std::vector<uint8_t>()));

  const Offset rr{offset};
  const Message msg{
      Type::kConsumeRequest, 0, rr.Encode()
  };

  std::shared_ptr<FakeConnection> conn = std::make_shared<FakeConnection>();

  const Message expected_response{
      Type::kConsumeResponse, 0, Record{}.Encode()
  };
  EXPECT_CALL(*responder, Respond(Event{expected_response, conn}));

  handler.Handle(Event{msg, conn});
}

TEST_F(ConsumeHandlerTest, HandleUnrecognizedType) {
  std::shared_ptr<MockResponder> responder = std::make_shared<MockResponder>();
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  ConsumeHandler handler{responder, log};

  std::shared_ptr<FakeConnection> conn = std::make_shared<FakeConnection>();
  const Message msg{
      Type::kProduceRequest, 0, {0, 1, 2, 3, 4}
  };

  handler.Handle(Event{msg, conn});
}

TEST_F(ConsumeHandlerTest, HandleInvalidOffset) {
  std::shared_ptr<MockResponder> responder = std::make_shared<MockResponder>();
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  ConsumeHandler handler{responder, log};

  std::shared_ptr<FakeConnection> conn = std::make_shared<FakeConnection>();
  const Message msg{
      Type::kConsumeRequest, 0, {0xff}
  };

  handler.Handle(Event{msg, conn});
}

}  // namespace wombat::broker
