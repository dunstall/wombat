// Copyright 2020 Andrew Dunstall

#include <memory>
#include <vector>

#include "broker/router.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "record/message.h"
#include "server/event.h"

namespace wombat::broker {

class MockPartition : public Partition {
 public:
  explicit MockPartition(uint32_t id) : Partition{id} {}

  MOCK_METHOD(void, Handle, (const server::Event& evt), (override));
};

class RouterTest : public ::testing::Test {
 protected:
  server::Event Event(uint32_t id) const;
};

TEST_F(RouterTest, RouteToSinglePartition) {
  const uint32_t id = 0xff00001;
  std::unique_ptr<MockPartition> partition
      = std::make_unique<MockPartition>(id);
  EXPECT_CALL(*partition, Handle(Event(id))).Times(1);

  Router router{};
  router.AddPartition(std::move(partition));
  EXPECT_TRUE(router.Route(Event(id)));
}

TEST_F(RouterTest, RoutedPartitionNotFound) {
  const uint32_t id = 0xff00001;
  Router router{};
  EXPECT_FALSE(router.Route(Event(id)));
}

TEST_F(RouterTest, RouteToMultiPartitions) {
  Router router{};
  for (uint32_t id = 0; id != 0xf; ++id) {
    std::unique_ptr<MockPartition> partition = std::make_unique<MockPartition>(
        id
    );
    EXPECT_CALL(*partition, Handle(Event(id))).Times(1);
    router.AddPartition(std::move(partition));
  }

  for (uint32_t id = 0; id != 0xf; ++id) {
    EXPECT_TRUE(router.Route(Event(id)));
  }
}

TEST_F(RouterTest, AddPartitionOverrides) {
  const uint32_t id = 0xff00001;

  std::unique_ptr<MockPartition> partition1 = std::make_unique<MockPartition>(
      id
  );
  EXPECT_CALL(*partition1, Handle(Event(id))).Times(1);
  std::unique_ptr<MockPartition> partition2 = std::make_unique<MockPartition>(
      id
  );
  EXPECT_CALL(*partition2, Handle(Event(id))).Times(1);

  Router router{};

  router.AddPartition(std::move(partition1));
  EXPECT_TRUE(router.Route(Event(id)));

  router.AddPartition(std::move(partition2));
  EXPECT_TRUE(router.Route(Event(id)));
}

server::Event RouterTest::Event(uint32_t id) const {
  const record::MessageType type = record::MessageType::kConsumeRequest;
  const std::vector<uint8_t> payload(0x200, 0xff);
  const record::Message msg{type, id, payload};
  return server::Event{msg, nullptr};
}

}  // namespace wombat::broker
