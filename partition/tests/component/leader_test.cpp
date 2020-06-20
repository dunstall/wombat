#include "gtest/gtest.h"

#include <chrono>
#include <thread>

#include "partition/leader.h"
#include "partition/replica.h"
#include "log/log.h"
#include "log/logexception.h"
#include "log/inmemorysegment.h"
#include "log/tempdir.h"

namespace wombat::log::testing {

class LeaderTest : public ::testing::Test {};

// TODO just start leader in another thread?
// TODO or just assume already running? - test leader with mix of replica and
// raw sockets (its fine for this to be a bit messy - cleanup later when more
// of the system is built)
// TODO or use python client?


TEST_F(LeaderTest, TestConnectOk) {
  TempDir dir{};
  std::shared_ptr<Log<InMemorySegment>> log = std::make_shared<Log<InMemorySegment>>(dir.path(), 128'000'000);
  
  const LeaderAddress addr{"127.0.0.1", 3110};
  Replica<InMemorySegment> replica1(log, addr);
  Replica<InMemorySegment> replica2(log, addr);
  Replica<InMemorySegment> replica3(log, addr);

  replica1.Poll();
  EXPECT_TRUE(replica1.connected());
  replica2.Poll();
  EXPECT_TRUE(replica2.connected());
  replica3.Poll();
  EXPECT_TRUE(replica3.connected());
}

TEST_F(LeaderTest, TestReceiveData) { 
  TempDir dir{};
  std::shared_ptr<Log<InMemorySegment>> log = std::make_shared<Log<InMemorySegment>>(dir.path(), 128'000'000);
  
  const LeaderAddress addr{"127.0.0.1", 3110};
  Replica<InMemorySegment> replica(log, addr);

  replica.Poll();
  EXPECT_TRUE(replica.connected());

  for (int i = 0; i != 10; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    replica.Poll();
  }

  EXPECT_EQ(std::vector<uint8_t>({0, 0, 0, 0, 0}), log->Lookup(0, 5));
  EXPECT_EQ(std::vector<uint8_t>({1, 1, 1, 1, 1}), log->Lookup(5, 5));
  EXPECT_EQ(std::vector<uint8_t>({2, 2, 2, 2, 2}), log->Lookup(10, 5));
  EXPECT_EQ(std::vector<uint8_t>({3, 3, 3, 3, 3}), log->Lookup(15, 5));
  EXPECT_EQ(std::vector<uint8_t>({4, 4, 4, 4, 4}), log->Lookup(20, 5));
  EXPECT_EQ(std::vector<uint8_t>({5, 5, 5, 5, 5}), log->Lookup(25, 5));

  std::cout << log->size() << std::endl;
}

// TODO connect - send one byte a second for offset

// TODO exceed max replicas

}  // namespace wombat::log::testing
