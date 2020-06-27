#include "gtest/gtest.h"

#include <chrono>
#include <thread>

#include "partition/replica.h"
#include "log/log.h"
#include "log/logexception.h"
#include "log/inmemorysegment.h"
#include "log/tempdir.h"

namespace wombat::broker::testing {

class ReplicaTest : public ::testing::Test {};

// TODO(AD) Must start test handler in background of each test.

// TEST_F(ReplicaTest, LeaderOk) {
  // TempDir dir{};
  // std::shared_ptr<Log<InMemorySegment>> log = std::make_shared<Log<InMemorySegment>>(dir.path(), 128'000'000);

  // const LeaderAddress addr{"127.0.0.1", 5100};
  // Replica<InMemorySegment> replica(log, addr);

  // while (log->size() < 25) {
    // replica.Poll();
    // EXPECT_TRUE(replica.connected());
  // }

  // for (int i = 0; i != 5; i++) {
    // uint32_t offset = i * 5;
    // EXPECT_EQ(std::vector<uint8_t>({4, 1, 2, 3, 4}), log->Lookup(offset, 5));
  // }
// }

// TEST_F(ReplicaTest, LeaderUnreachable) {
  // TempDir dir{};
  // std::shared_ptr<Log<InMemorySegment>> log = std::make_shared<Log<InMemorySegment>>(dir.path(), 128'000'000);

  // const LeaderAddress addr{"127.0.0.1", 5101};
  // Replica<InMemorySegment> replica(log, addr);

  // replica.Poll();

  // EXPECT_FALSE(replica.connected());
// }

// TEST_F(ReplicaTest, LeaderCloseImmediately) {
  // TempDir dir{};
  // std::shared_ptr<Log<InMemorySegment>> log = std::make_shared<Log<InMemorySegment>>(dir.path(), 128'000'000);

  // const LeaderAddress addr{"127.0.0.1", 5102};
  // Replica<InMemorySegment> replica(log, addr);

  // replica.Poll();
  // EXPECT_FALSE(replica.connected());
// }

// TEST_F(ReplicaTest, LeaderWriteAndClose) {
  // TempDir dir{};
  // std::shared_ptr<Log<InMemorySegment>> log = std::make_shared<Log<InMemorySegment>>(dir.path(), 128'000'000);

  // const LeaderAddress addr{"127.0.0.1", 5103};
  // Replica<InMemorySegment> replica(log, addr);

  // while (log->size() < 5) {
    // replica.Poll();
  // }

  // EXPECT_EQ(std::vector<uint8_t>({4, 1, 2, 3, 4}), log->Lookup(0, 5));
/* } */

// TODO(AD) server sleep 1 s before sends

// TODO(AD) server never accepts connection (but is listening on port)

}  // namespace wombat::broker::testing
