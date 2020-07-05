// Copyright 2020 Andrew Dunstall

#include <cstdint>
#include <memory>
#include <random>
#include <thread>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "partition/leader.h"
#include "record/request.h"
#include "record/recordrequest.h"
#include "tmp/log.h"
#include "tmp/server.h"
#include "util/threadable.h"

namespace wombat::broker::partition::testing {

class LeaderTest : public ::testing::Test {};

class FakeServer : public Server {
 public:
  ~FakeServer() override {}

  void Poll() override {};
};

class MockLog : public Log {
 public:
  MOCK_METHOD(void, Append, (const std::vector<uint8_t>& data), (override));
  MOCK_METHOD(std::vector<uint8_t>, Lookup, (uint32_t offset, uint32_t size), (override));  // NOLINT
  MOCK_METHOD(uint32_t, Send, (uint32_t offset, uint32_t size, int fd), (override));  // NOLINT
};

TEST_F(LeaderTest, TestRequestSingleReplica) {
  const uint32_t offset = 0;
  const record::RecordRequest rr{offset};
  const record::Request request{record::RequestType::kReplica, rr.Encode()};

  const int fd = 10;
  std::unique_ptr<Server> server = std::make_unique<FakeServer>();
  // As server is moved and owned by leader get a pointer (this is hacky but
  // ok for testing as we know the lifecycle).
  Server* ptr = server.get();
  ptr->events()->Push(Event{request, std::make_shared<Connection>(fd)});

  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  // Initially send offset and keep attempting to send more data starting
  // at the latest non-sent offset.
  const uint32_t sent1 = 0xff;
  EXPECT_CALL(*log, Send(offset, 1024, fd)).WillOnce(::testing::Return(sent1));
  const uint32_t sent2 = 0xfa;
  EXPECT_CALL(*log, Send(offset + sent1, 1024, fd))
      .WillOnce(::testing::Return(sent2));
  EXPECT_CALL(*log, Send(offset + sent1 + sent2, 1024, fd))
      .WillOnce(::testing::Return(0xa));

  Leader leader(std::move(server), log);
  leader.Poll();
  leader.Poll();
  leader.Poll();

  const uint32_t offset_new = 0xaa;
  const record::RecordRequest rr_new{offset_new};
  const record::Request request_new{
      record::RequestType::kReplica, rr_new.Encode()
  };
  // Reconnect the replica with a new offset.
  ptr->events()->Push(Event{request_new, std::make_shared<Connection>(fd)});

  EXPECT_CALL(*log, Send(offset_new, 1024, fd))
      .WillOnce(::testing::Return(0xa));
  leader.Poll();
}

TEST_F(LeaderTest, TestRequestMultiReplica) {
  const uint32_t offset1 = 0;
  const int fd1 = 7;
  const record::RecordRequest rr1{offset1};
  const record::Request request1{record::RequestType::kReplica, rr1.Encode()};

  const uint32_t offset2 = 0xaa;
  const int fd2 = 8;
  const record::RecordRequest rr2{offset2};
  const record::Request request2{record::RequestType::kReplica, rr2.Encode()};

  const uint32_t offset3 = 0xff;
  const int fd3 = 10;
  const record::RecordRequest rr3{offset3};
  const record::Request request3{record::RequestType::kReplica, rr3.Encode()};

  std::unique_ptr<Server> server = std::make_unique<FakeServer>();
  // As server is moved and owned by leader get a pointer (this is hacky but
  // ok for testing as we know the lifecycle).
  Server* ptr = server.get();
  ptr->events()->Push(Event{request1, std::make_shared<Connection>(fd1)});
  ptr->events()->Push(Event{request2, std::make_shared<Connection>(fd2)});
  ptr->events()->Push(Event{request3, std::make_shared<Connection>(fd3)});

  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  EXPECT_CALL(*log, Send(offset1, 1024, fd1)).WillOnce(::testing::Return(0xff));
  EXPECT_CALL(*log, Send(offset2, 1024, fd2)).WillOnce(::testing::Return(0xff));
  EXPECT_CALL(*log, Send(offset3, 1024, fd3)).WillOnce(::testing::Return(0xff));

  Leader leader(std::move(server), log);
  leader.Poll();
}

TEST_F(LeaderTest, TestReplicaDisconnect) {
  const uint32_t offset = 0;
  const record::RecordRequest rr{offset};
  const record::Request request{record::RequestType::kReplica, rr.Encode()};

  const int fd = 10;
  std::unique_ptr<Server> server = std::make_unique<FakeServer>();
  // As server is moved and owned by leader get a pointer (this is hacky but
  // ok for testing as we know the lifecycle).
  Server* ptr = server.get();
  std::shared_ptr<Connection> conn = std::make_shared<Connection>(fd);
  ptr->events()->Push(Event{request, conn});

  std::shared_ptr<MockLog> log = std::make_shared<MockLog>();
  // Initially send offset and keep attempting to send more data starting
  // at the latest non-sent offset.
  EXPECT_CALL(*log, Send(offset, 1024, fd)).WillOnce(::testing::Return(0xaa));

  Leader leader(std::move(server), log);
  leader.Poll();

  conn->close();

  // As the connection is closed this should not call send again.
  leader.Poll();
}

TEST_F(LeaderTest, TestInvalidRequestRecord) {
  record::Request request{record::RequestType::kReplica, {1, 2}};

  std::unique_ptr<Server> server = std::make_unique<FakeServer>();
  server->events()->Push(Event{request, std::make_shared<Connection>(0)});

  std::shared_ptr<Log> log = std::make_shared<MockLog>();

  Leader leader(std::move(server), log);
  // Should ignore the bad request.
  leader.Poll();
}

TEST_F(LeaderTest, TestUnrecognizedRequest) {
  record::RecordRequest rr{0};
  // kProduce in not recognized by the leader.
  record::Request request{record::RequestType::kProduce, rr.Encode()};

  std::unique_ptr<Server> server = std::make_unique<FakeServer>();
  server->events()->Push(Event{request, std::make_shared<Connection>(0)});

  std::shared_ptr<Log> log = std::make_shared<MockLog>();

  Leader leader(std::move(server), log);
  // Should ignore the bad request.
  leader.Poll();
}

}  // namespace wombat::broker::partition::testing
