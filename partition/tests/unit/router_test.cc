// Copyright 2020 Andrew Dunstall

#include <memory>
#include <optional>

#include "event/event.h"
#include "event/responder.h"
#include "frame/message.h"
#include "frame/messageheader.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "partition/handler.h"
#include "partition/router.h"

namespace wombat::broker::partition {

class MockHandler : public Handler {
 public:
  MockHandler() : Handler(0) {}

  MOCK_METHOD(std::optional<Event>, Handle, (const Event& evt), (override));
};

class MockResponder : public Responder {
 public:
  MOCK_METHOD(void, Respond, (const Event& evt), (override));
};

class RouterTest : public ::testing::Test {
 public:
  const uint32_t kPartitionId = 0xffaa;
};

TEST_F(RouterTest, RouteToFoundHandler) {
  const frame::Message msg{frame::Type::kProduceRequest, kPartitionId, {}};
  const Event evt{msg, nullptr};

  std::unique_ptr<MockHandler> handler = std::make_unique<MockHandler>();

  const frame::Message msg_resp{frame::Type::kStatResponse, kPartitionId, {}};
  const Event evt_resp{msg, nullptr};
  EXPECT_CALL(*handler, Handle(evt)).WillOnce(::testing::Return(evt_resp));

  std::shared_ptr<MockResponder> responder = std::make_shared<MockResponder>();
  EXPECT_CALL(*responder, Respond(evt_resp)).Times(1);

  Router router{responder};
  // Add redundent handlers that should be ignored.
  router.AddRoute(
      frame::Type::kConsumeRequest, std::make_unique<MockHandler>()
  );
  router.AddRoute(frame::Type::kProduceRequest, std::move(handler));
  router.AddRoute(
      frame::Type::kConsumeResponse, std::make_unique<MockHandler>()
  );
  router.Route(evt);
}

TEST_F(RouterTest, RouteToFoundHandlerNoResponse) {
  const frame::Message msg{frame::Type::kProduceRequest, kPartitionId, {}};
  const Event evt{msg, nullptr};

  std::unique_ptr<MockHandler> handler = std::make_unique<MockHandler>();

  EXPECT_CALL(*handler, Handle(evt)).WillOnce(::testing::Return(std::nullopt));

  std::shared_ptr<MockResponder> responder = std::make_shared<MockResponder>();

  Router router{responder};
  // Add redundent handlers that should be ignored.
  router.AddRoute(
      frame::Type::kConsumeRequest, std::make_unique<MockHandler>()
  );
  router.AddRoute(frame::Type::kProduceRequest, std::move(handler));
  router.AddRoute(
      frame::Type::kConsumeResponse, std::make_unique<MockHandler>()
  );
  router.Route(evt);
}

TEST_F(RouterTest, RouteToNotFoundHandler) {
  const frame::Message msg{frame::Type::kProduceRequest, kPartitionId, {}};
  const Event evt{msg, nullptr};

  std::unique_ptr<MockHandler> handler = std::make_unique<MockHandler>();

  Router router{nullptr};
  // Add handler with different type to the message.
  router.AddRoute(frame::Type::kConsumeRequest, std::move(handler));
  router.Route(evt);
}

}  // namespace wombat::broker::partition
