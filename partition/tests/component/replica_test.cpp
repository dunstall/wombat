#include "gtest/gtest.h"

#include <chrono>
#include <thread>

#include "log/log.h"
#include "log/logexception.h"
#include "log/inmemorysegment.h"
#include "log/replica.h"
#include "log/tempdir.h"

namespace wombat::log::testing {

class ReplicaTest : public ::testing::Test {};

TEST_F(ReplicaTest, LeaderLogEmpty) {
  TempDir dir{};

  Log<InMemorySegment> log(dir.path(), 128'000'000);

  const LeaderAddress addr{"127.0.0.1", 5100};
  Replica<InMemorySegment> replica(log, addr);

  for (int i = 0; i != 10; ++i) {
    replica.Poll();
  }

  EXPECT_EQ(std::vector<uint8_t>({4, 1, 1, 1, 1}), log.Lookup(0, 5));
}

TEST_F(ReplicaTest, LeaderUnreachable) {
  FAIL();
}

TEST_F(ReplicaTest, LeaderCloseImmediately) {
  TempDir dir{};

  const LeaderAddress addr{"127.0.0.1", 5102};
  Replica<InMemorySegment> replica(
      Log<InMemorySegment>(dir.path(), 128'000'000), addr
  );
}

// Server sends 1 byte and closes the connection.
TEST_F(ReplicaTest, LeaderWriteByteAndClose) {
  TempDir dir{};

  Log<InMemorySegment> log(dir.path(), 128'000'000);

  const LeaderAddress addr{"127.0.0.1", 5103};
  Replica<InMemorySegment> replica(log, addr);

  for (int i = 0; i != 10; ++i) {
    replica.Poll();
  }

  EXPECT_EQ(std::vector<uint8_t>({4, 1, 1, 1, 1}), log.Lookup(0, 5));
}

// TODO(AD) server sleep 1 s before sends

// TODO(AD) server never accepts connection (but is listening on port)

}  // namespace wombat::log::testing
