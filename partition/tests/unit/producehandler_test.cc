// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <vector>

#include "frame/message.h"
#include "frame/record.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "log/log.h"
#include "partition/producehandler.h"

namespace wombat::broker {

class MockLog : public log::Log {
 public:
  MOCK_METHOD(void, Append, (const std::vector<uint8_t>& data), (override));
  MOCK_METHOD(std::vector<uint8_t>, Lookup, (uint32_t offset, uint32_t size), (override));  // NOLINT
};

class ProduceHandlerTest : public ::testing::Test {};

TEST_F(ProduceHandlerTest, HandleValidProduceRequest) {
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  ProduceHandler handler{log};

  const std::vector<uint8_t> data{1, 2, 3, 4, 5};
  const record::Record record{data};
  const record::Message msg{
      record::MessageType::kProduceRequest, 0, record.Encode()
  };

  EXPECT_CALL(*log, Append(record.Encode())).Times(1);

  handler.Handle(msg);
}

TEST_F(ProduceHandlerTest, HandleInvalidType) {
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  ProduceHandler handler{log};

  const record::Message msg{
      record::MessageType::kConsumeRequest, 0, {0, 1, 2, 3, 4}
  };

  handler.Handle(msg);
}

TEST_F(ProduceHandlerTest, HandleInvalidRecord) {
  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  ProduceHandler handler{log};

  const record::Message msg{
      record::MessageType::kProduceRequest, 0, {0xff, 1, 2, 3, 4}
  };

  handler.Handle(msg);
}

}  // namespace wombat::broker
