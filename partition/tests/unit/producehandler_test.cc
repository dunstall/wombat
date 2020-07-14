// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <optional>
#include <vector>

#include "frame/message.h"
#include "frame/record.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "log/log.h"
#include "log/mocklog.h"
#include "partition/producehandler.h"

namespace wombat::broker::partition {

class ProduceHandlerTest : public ::testing::Test {
 protected:
  const uint32_t kPartitionId = 0xffaa;
};

TEST_F(ProduceHandlerTest, HandleValidProduceRequest) {
  std::shared_ptr<log::MockLog> log = std::make_shared<log::MockLog>();
  ProduceHandler handler{kPartitionId, log};

  const std::vector<uint8_t> data{1, 2, 3, 4, 5};
  const frame::Record record{data};
  const frame::Message msg{frame::Type::kProduceRequest, kPartitionId,
                           record.Encode()};

  EXPECT_CALL(*log, Append(record.Encode())).Times(1);
  EXPECT_EQ(std::nullopt, handler.Handle(Event{msg, nullptr}));
}

TEST_F(ProduceHandlerTest, HandleInvalidType) {
  ProduceHandler handler{kPartitionId, nullptr};
  const frame::Message msg{frame::Type::kConsumeRequest, kPartitionId, {}};
  EXPECT_EQ(std::nullopt, handler.Handle(Event{msg, nullptr}));
}

TEST_F(ProduceHandlerTest, HandleInvalidRecord) {
  ProduceHandler handler{kPartitionId, nullptr};
  const frame::Message msg{frame::Type::kProduceRequest, kPartitionId, {}};
  EXPECT_EQ(std::nullopt, handler.Handle(Event{msg, nullptr}));
}

}  // namespace wombat::broker::partition
