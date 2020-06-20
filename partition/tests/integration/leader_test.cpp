#include "gtest/gtest.h"

#include <atomic>
#include <chrono>
#include <thread>

#include "partition/leader.h"
#include "partition/replica.h"
#include "log/log.h"
#include "log/logexception.h"
#include "log/inmemorysegment.h"
#include "log/tempdir.h"

namespace wombat::log::testing {

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
  const uint16_t port = 12345;

  TempDir leader_dir{};
  std::shared_ptr<Log<InMemorySegment>> leader_log
      = std::make_shared<Log<InMemorySegment>>(leader_dir.path(), kSegmentLimit);

  LeaderServer server{leader_log, port};
  server.Start();

  const LeaderAddress addr{kLocalhost, port};

  TempDir replica_dir{};
  std::shared_ptr<Log<InMemorySegment>> replica_log
      = std::make_shared<Log<InMemorySegment>>(replica_dir.path(), kSegmentLimit);
  
  Replica<InMemorySegment> replica1(replica_log, addr);
  Replica<InMemorySegment> replica2(replica_log, addr);
  Replica<InMemorySegment> replica3(replica_log, addr);

  replica1.Poll();
  replica2.Poll();
  replica3.Poll();

  EXPECT_TRUE(replica1.connected());
  EXPECT_TRUE(replica2.connected());
  EXPECT_TRUE(replica3.connected());

  server.Stop();
}

// TODO connect exceed limit (make param?)

// TEST_F(LeaderTest, TestReceiveDataOffset0) { 
  // TempDir dir{};
  // std::shared_ptr<Log<InMemorySegment>> log = std::make_shared<Log<InMemorySegment>>(dir.path(), 128'000'000);
  
  // const LeaderAddress addr{"127.0.0.1", 3110};
  // Replica<InMemorySegment> replica(log, addr);

  // replica.Poll();
  // EXPECT_TRUE(replica.connected());

  // while (log->size() < 25) {
    // replica.Poll();
  // }

  // for (uint8_t i = 0; i != 5; ++i) {
    // EXPECT_EQ(std::vector<uint8_t>({i, i, i, i, i}), log->Lookup(i * 5, 5));
  // }

  // std::cout << log->size() << std::endl;
// }

// // TODO setup by starting new leader and tear down at end
// //
// // want to be able to pass a log to the leader to read from
// TEST_F(LeaderTest, TestReceiveDataOffset25) { 
  // TempDir dir{};
  // std::shared_ptr<Log<InMemorySegment>> log = std::make_shared<Log<InMemorySegment>>(dir.path(), 128'000'000);
  // for (uint8_t b = 0; b != 0x5; ++b) {
    // log->Append({b, b, b, b, b});
  // }
  
  // const LeaderAddress addr{"127.0.0.1", 3110};
  // Replica<InMemorySegment> replica(log, addr);

  // replica.Poll();
  // EXPECT_TRUE(replica.connected());

  // while (log->size() < 50) {
    // replica.Poll();
  // }

  // for (uint8_t i = 5; i != 10; ++i) {
    // EXPECT_EQ(std::vector<uint8_t>({i, i, i, i, i}), log->Lookup(i * 5, 5));
  // }

  // std::cout << log->size() << std::endl;
/* } */

// TODO connect - send one byte a second for offset

// TODO exceed max replicas

}  // namespace wombat::log::testing
