// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <optional>
#include <vector>

#include "event/event.h"
#include "frame/message.h"
#include "frame/offset.h"
#include "frame/record.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "log/log.h"
#include "log/mocklog.h"
#include "partition/consumehandler.h"

namespace wombat::broker::partition {

class FakeConnection : public Connection {
 public:
  std::optional<frame::Message> Receive() override { return std::nullopt; }

  bool Send(const frame::Message& msg) override { return false; }
};

class ConsumeHandlerTest : public ::testing::Test {
 public:
  const uint32_t kPartitionId = 0xffaa;
  const uint32_t kOffset = 0xffaa;
};

TEST_F(ConsumeHandlerTest, HandleValidConsumeRequest) {
  std::shared_ptr<log::MockLog> log = std::make_shared<log::MockLog>();
  ConsumeHandler handler{kPartitionId, log};

  const std::vector<uint8_t> payload{1, 2, 3, 4, 5};
  const frame::Record record{payload};
  const std::vector<uint8_t> encoded = record.Encode();
  const std::vector<uint8_t> encoded_size(encoded.begin(), encoded.begin() + 4);

  // Expect to first lookup the record size then the full record.
  EXPECT_CALL(*log, Lookup(kOffset, sizeof(uint32_t)))
      .WillOnce(::testing::Return(encoded_size));
  EXPECT_CALL(*log, Lookup(kOffset, encoded.size()))
      .WillOnce(::testing::Return(encoded));

  const frame::Offset offset{kOffset};
  const frame::Message msg{frame::Type::kConsumeRequest, kPartitionId,
                           offset.Encode()};

  std::shared_ptr<FakeConnection> conn = std::make_shared<FakeConnection>();
  const frame::Message expected_response{frame::Type::kConsumeResponse,
                                         kPartitionId, record.Encode()};
  const Event expected_event{expected_response, conn};
  EXPECT_EQ(expected_event, handler.Handle(Event{msg, conn}));
}

TEST_F(ConsumeHandlerTest, HandleOffsetExceedsLogSize) {
  std::shared_ptr<log::MockLog> log = std::make_shared<log::MockLog>();
  ConsumeHandler handler{kPartitionId, log};

  EXPECT_CALL(*log, Lookup(kOffset, sizeof(uint32_t)))
      .WillOnce(::testing::Return(std::vector<uint8_t>()));

  const frame::Offset offset{kOffset};
  const frame::Message msg{frame::Type::kConsumeRequest, kPartitionId,
                           offset.Encode()};

  std::shared_ptr<FakeConnection> conn = std::make_shared<FakeConnection>();
  const frame::Message expected_response{
      frame::Type::kConsumeResponse, kPartitionId, frame::Record{}.Encode()};
  const Event expected_event{expected_response, conn};
  EXPECT_EQ(expected_event, handler.Handle(Event{msg, conn}));
}

TEST_F(ConsumeHandlerTest, HandleUnrecognizedRequestType) {
  ConsumeHandler handler{kPartitionId, nullptr};

  const frame::Message msg{frame::Type::kProduceRequest, kPartitionId, {}};
  EXPECT_EQ(std::nullopt, handler.Handle(Event{msg, nullptr}));
}

TEST_F(ConsumeHandlerTest, HandleInvalidRequest) {
  ConsumeHandler handler{kPartitionId, nullptr};

  const frame::Message msg{frame::Type::kConsumeRequest, kPartitionId, {}};
  EXPECT_EQ(std::nullopt, handler.Handle(Event{msg, nullptr}));
}

}  // namespace wombat::broker::partition
