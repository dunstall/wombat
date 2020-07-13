// Copyright 2020 Andrew Dunstall

#include <memory>
#include <vector>

#include "broker/router.h"
#include "event/event.h"
#include "frame/message.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "partition/partition.h"

namespace wombat::broker {

class MockPartition : public partition::Partition {
 public:
  explicit MockPartition(uint32_t id) : partition::Partition{id, nullptr} {}

  MOCK_METHOD(void, Handle, (const Event& evt), (override));

  void Process() override {}
};

class RouterTest : public ::testing::Test {
 protected:
  Event CreateEvent(uint32_t id) const;
};

TEST_F(RouterTest, RouteToSinglePartition) {
  const uint32_t id = 0xff00001;
  std::unique_ptr<MockPartition> partition =
      std::make_unique<MockPartition>(id);
  EXPECT_CALL(*partition, Handle(CreateEvent(id))).Times(1);

  Router router{};
  router.AddPartition(std::move(partition));
  EXPECT_TRUE(router.Route(CreateEvent(id)));
}

TEST_F(RouterTest, RoutedPartitionNotFound) {
  const uint32_t id = 0xff00001;
  Router router{};
  EXPECT_FALSE(router.Route(CreateEvent(id)));
}

TEST_F(RouterTest, RouteToMultiPartitions) {
  Router router{};
  for (uint32_t id = 0; id != 0xf; ++id) {
    std::unique_ptr<MockPartition> partition =
        std::make_unique<MockPartition>(id);
    EXPECT_CALL(*partition, Handle(CreateEvent(id))).Times(1);
    router.AddPartition(std::move(partition));
  }

  for (uint32_t id = 0; id != 0xf; ++id) {
    EXPECT_TRUE(router.Route(CreateEvent(id)));
  }
}

TEST_F(RouterTest, AddPartitionOverrides) {
  const uint32_t id = 0xff00001;

  std::unique_ptr<MockPartition> partition1 =
      std::make_unique<MockPartition>(id);
  EXPECT_CALL(*partition1, Handle(CreateEvent(id))).Times(1);
  std::unique_ptr<MockPartition> partition2 =
      std::make_unique<MockPartition>(id);
  EXPECT_CALL(*partition2, Handle(CreateEvent(id))).Times(1);

  Router router{};

  router.AddPartition(std::move(partition1));
  EXPECT_TRUE(router.Route(CreateEvent(id)));

  router.AddPartition(std::move(partition2));
  EXPECT_TRUE(router.Route(CreateEvent(id)));
}

Event RouterTest::CreateEvent(uint32_t id) const {
  const frame::Type type = frame::Type::kConsumeRequest;
  const std::vector<uint8_t> payload(0x200, 0xff);
  const frame::Message msg{type, id, payload};
  return Event{msg, nullptr};
}

}  // namespace wombat::broker
