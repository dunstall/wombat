#include "gtest/gtest.h"

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>

#include "partition/leader.h"
#include "partition/replica.h"
#include "log/log.h"
#include "log/systemsegment.h"
#include "log/tempdir.h"

namespace wombat::broker::testing {

// TODO repeat this with test harness for replica tests
template<class S>
class LeaderServer {
 public:
  LeaderServer(std::shared_ptr<Log<S>> log, uint16_t port)
      : leader_{log, port} {}

  void Start() {
    running_ = true;
    thread_ = std::thread{&LeaderServer::Poll, this};
  }

  void Stop() {
    running_ = false;
    if (thread_.joinable()) {
      thread_.join();
    }
  }

 private:
  void Poll() {
    while (running_) {
      leader_.Poll();
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }

  Leader<S> leader_;

  std::thread thread_;
  std::atomic_bool running_;
};

class LeaderTest : public ::testing::Test {
 protected:
  const std::string kLocalhost = "127.0.0.1";
  const uint32_t kSegmentLimit = 128'000'000;
};

TEST_F(LeaderTest, TestConnectOk) {
  const uint16_t port = 3100;

  TempDir leader_dir{};
  std::shared_ptr<Log<SystemSegment>> leader_log
      = std::make_shared<Log<SystemSegment>>(leader_dir.path(), kSegmentLimit);

  LeaderServer server{leader_log, port};
  server.Start();

  TempDir replica_dir{};
  std::shared_ptr<Log<SystemSegment>> replica_log
      = std::make_shared<Log<SystemSegment>>(replica_dir.path(), kSegmentLimit);

  const LeaderAddress addr{kLocalhost, port};
  for (int i = 0; i != 5; ++i) {
    Replica<SystemSegment> replica(replica_log, addr);
    replica.Poll();
    EXPECT_TRUE(replica.connected());
  }

  server.Stop();
}

TEST_F(LeaderTest, TestConnectExceedReplicaLimit) {
  const uint16_t port = 3101;

  TempDir leader_dir{};
  std::shared_ptr<Log<SystemSegment>> leader_log
      = std::make_shared<Log<SystemSegment>>(leader_dir.path(), kSegmentLimit);

  LeaderServer server{leader_log, port};
  server.Start();

  TempDir replica_dir{};
  std::shared_ptr<Log<SystemSegment>> replica_log
      = std::make_shared<Log<SystemSegment>>(replica_dir.path(), kSegmentLimit);

  const LeaderAddress addr{kLocalhost, port};
  // Keep replicas in memory to avoid closing.
  std::vector<Replica<SystemSegment>> replicas{};
  for (int i = 0; i != 10; ++i) {
    Replica<SystemSegment> replica(replica_log, addr);
    replica.Poll();
    EXPECT_TRUE(replica.connected());
    replicas.push_back(std::move(replica));
  }

  // Exceed the limit.
  Replica<SystemSegment> replica(replica_log, addr);
  replica.Poll();
  // Connection should fail.
  EXPECT_FALSE(replica.connected());

  server.Stop();
}

// Open N > kMaxReplicas replica but close each so should be able to exceed the
// limit.
TEST_F(LeaderTest, TestConnectDisconnectedReplicaExceedLimit) {
  const uint16_t port = 31011;

  TempDir leader_dir{};
  std::shared_ptr<Log<SystemSegment>> leader_log
      = std::make_shared<Log<SystemSegment>>(leader_dir.path(), kSegmentLimit);

  LeaderServer server{leader_log, port};
  server.Start();

  TempDir replica_dir{};
  std::shared_ptr<Log<SystemSegment>> replica_log
      = std::make_shared<Log<SystemSegment>>(replica_dir.path(), kSegmentLimit);

  const LeaderAddress addr{kLocalhost, port};
  // Keep replicas in memory to avoid closing.
  for (int i = 0; i != 10; ++i) {
    Replica<SystemSegment> replica(replica_log, addr);
    replica.Poll();
    EXPECT_TRUE(replica.connected());
  }

  // Exceed the limit.
  Replica<SystemSegment> replica(replica_log, addr);
  replica.Poll();
  // Connection should succeed as other replicas have closed.
  EXPECT_TRUE(replica.connected());

  server.Stop();
}

TEST_F(LeaderTest, TestReceiveDataOffsetZero) {
  const uint16_t port = 3104;

  const std::vector<uint8_t> data{1, 2, 3, 4, 5};

  TempDir leader_dir{};
  std::shared_ptr<Log<SystemSegment>> leader_log
      = std::make_shared<Log<SystemSegment>>(leader_dir.path(), kSegmentLimit);
  leader_log->Append(data);

  LeaderServer server{leader_log, port};
  server.Start();

  TempDir replica_dir{};
  std::shared_ptr<Log<SystemSegment>> replica_log
      = std::make_shared<Log<SystemSegment>>(replica_dir.path(), kSegmentLimit);

  const LeaderAddress addr{kLocalhost, port};
  Replica<SystemSegment> replica(replica_log, addr);

  while (replica_log->size() < data.size()) {
    replica.Poll();
    ASSERT_TRUE(replica.connected());
  }

  EXPECT_EQ(data.size(), replica_log->size());
  EXPECT_EQ(data, replica_log->Lookup(0, data.size()));

  server.Stop();
}

TEST_F(LeaderTest, TestAppendToLeader) {
  const uint16_t port = 3105;

  TempDir leader_dir{};
  std::shared_ptr<Log<SystemSegment>> leader_log
      = std::make_shared<Log<SystemSegment>>(leader_dir.path(), kSegmentLimit);

  LeaderServer server{leader_log, port};
  server.Start();

  TempDir replica_dir{};
  std::shared_ptr<Log<SystemSegment>> replica_log
      = std::make_shared<Log<SystemSegment>>(replica_dir.path(), kSegmentLimit);

  const LeaderAddress addr{kLocalhost, port};
  Replica<SystemSegment> replica(replica_log, addr);

  for (int i = 0; i != 5; ++i) {
    replica.Poll();
    ASSERT_TRUE(replica.connected());
  }

  EXPECT_EQ(0U, replica_log->size());

  // Append to log after the replica has started streaming.
  const std::vector<uint8_t> data{1, 2, 3, 4, 5};
  leader_log->Append(data);

  while (replica_log->size() < data.size()) {
    replica.Poll();
    ASSERT_TRUE(replica.connected());
  }

  EXPECT_EQ(data.size(), replica_log->size());
  EXPECT_EQ(data, replica_log->Lookup(0, data.size()));

  server.Stop();
}

TEST_F(LeaderTest, TestReceiveDataOffsetNonZero) {
  const uint16_t port = 3106;

  const std::vector<uint8_t> data1{1, 2, 3, 4, 5};
  const std::vector<uint8_t> data2{6, 7, 8, 9, 10};

  TempDir leader_dir{};
  std::shared_ptr<Log<SystemSegment>> leader_log
      = std::make_shared<Log<SystemSegment>>(leader_dir.path(), kSegmentLimit);
  leader_log->Append(data1);
  leader_log->Append(data2);

  LeaderServer server{leader_log, port};
  server.Start();

  TempDir replica_dir{};
  std::shared_ptr<Log<SystemSegment>> replica_log
      = std::make_shared<Log<SystemSegment>>(replica_dir.path(), kSegmentLimit);
  // Start replica at offset 5.
  replica_log->Append(data1);

  const LeaderAddress addr{kLocalhost, port};
  Replica<SystemSegment> replica(replica_log, addr);

  while (replica_log->size() < leader_log->size()) {
    replica.Poll();
    ASSERT_TRUE(replica.connected());
  }

  EXPECT_EQ(leader_log->size(), replica_log->size());
  EXPECT_EQ(data1, replica_log->Lookup(0, data1.size()));
  EXPECT_EQ(data2, replica_log->Lookup(5, data2.size()));

  server.Stop();
}

TEST_F(LeaderTest, TestReplicasDifferentOffsets) {
  const uint16_t port = 3109;

  const std::vector<uint8_t> data1{1, 2, 3, 4, 5};
  const std::vector<uint8_t> data2{6, 7, 8, 9, 10};
  const std::vector<uint8_t> data3{11, 12, 13, 14, 15};

  TempDir leader_dir{};
  std::shared_ptr<Log<SystemSegment>> leader_log
      = std::make_shared<Log<SystemSegment>>(leader_dir.path(), kSegmentLimit);
  leader_log->Append(data1);
  leader_log->Append(data2);
  leader_log->Append(data3);

  LeaderServer server{leader_log, port};
  server.Start();

  TempDir replica1_dir{};
  std::shared_ptr<Log<SystemSegment>> replica1_log
      = std::make_shared<Log<SystemSegment>>(replica1_dir.path(), kSegmentLimit);
  // Start replica at offset 5.
  replica1_log->Append(data1);

  TempDir replica2_dir{};
  // Start replica at offset 0.
  std::shared_ptr<Log<SystemSegment>> replica2_log
      = std::make_shared<Log<SystemSegment>>(replica2_dir.path(), kSegmentLimit);

  TempDir replica3_dir{};
  std::shared_ptr<Log<SystemSegment>> replica3_log
      = std::make_shared<Log<SystemSegment>>(replica3_dir.path(), kSegmentLimit);
  // Start replica at offset 10.
  replica3_log->Append(data1);
  replica3_log->Append(data2);

  const LeaderAddress addr{kLocalhost, port};
  Replica<SystemSegment> replica1(replica1_log, addr);
  Replica<SystemSegment> replica2(replica2_log, addr);
  Replica<SystemSegment> replica3(replica3_log, addr);

  while (replica1_log->size() < leader_log->size()) {
    replica1.Poll();
    ASSERT_TRUE(replica1.connected());
  }

  while (replica2_log->size() < leader_log->size()) {
    replica2.Poll();
    ASSERT_TRUE(replica2.connected());
  }

  while (replica3_log->size() < leader_log->size()) {
    replica3.Poll();
    ASSERT_TRUE(replica3.connected());
  }

  EXPECT_EQ(leader_log->size(), replica1_log->size());
  EXPECT_EQ(data1, replica1_log->Lookup(0, data1.size()));
  EXPECT_EQ(data2, replica1_log->Lookup(5, data2.size()));
  EXPECT_EQ(data3, replica1_log->Lookup(10, data3.size()));

  EXPECT_EQ(leader_log->size(), replica1_log->size());
  EXPECT_EQ(data1, replica2_log->Lookup(0, data1.size()));
  EXPECT_EQ(data2, replica2_log->Lookup(5, data2.size()));
  EXPECT_EQ(data3, replica2_log->Lookup(10, data3.size()));

  EXPECT_EQ(leader_log->size(), replica1_log->size());
  EXPECT_EQ(data1, replica3_log->Lookup(0, data1.size()));
  EXPECT_EQ(data2, replica3_log->Lookup(5, data2.size()));
  EXPECT_EQ(data3, replica3_log->Lookup(10, data3.size()));

  server.Stop();
}

// TODO(AD) Slow connect - send one byte of the offset a second.

}  // namespace wombat::broker::testing
