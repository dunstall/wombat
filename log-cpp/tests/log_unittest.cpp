#include <vector>

#include "gtest/gtest.h"
#include "log/log.h"
#include "log/inmemorysegment.h"
#include "tempdir.h"

namespace wombat::log::testing {

class LogTest : public ::testing::Test {};

TEST_F(LogTest, OpenEmpty) {
  Log<InMemorySegment> log{GeneratePath(), 3};

  EXPECT_TRUE(log.Lookup(0U, 3U).empty());
}

TEST_F(LogTest, OpenExisting) {
  const auto path = GeneratePath();

  {
    Log<InMemorySegment> log{path, 3};
    log.Append({1, 2, 3, 4});  // Segment 1.
    log.Append({5, 6, 7, 8});  // Segment 2.
    log.Append({9, 10});  // Segment 3.
    log.Append({11, 12});  // Segment 3.
  }
  {
    Log<InMemorySegment> log{path, 3};
    EXPECT_EQ(std::vector<uint8_t>({1, 2, 3, 4}), log.Lookup(0, 4));  // Segment 1.
    EXPECT_EQ(std::vector<uint8_t>({5, 6, 7, 8}), log.Lookup(4, 4));  // Segment 2.
    EXPECT_EQ(std::vector<uint8_t>({9, 10, 11, 12}), log.Lookup(8, 4));  // Segment 3.
  }
  {
    InMemorySegment segment1{0x1, path, 3};
    EXPECT_EQ(std::vector<uint8_t>({1, 2, 3, 4}), segment1.Lookup(0, 4));
    InMemorySegment segment2{0x2, path, 3};
    EXPECT_EQ(std::vector<uint8_t>({5, 6, 7, 8}), segment2.Lookup(0, 4));
    InMemorySegment segment3{0x3, path, 3};
    EXPECT_EQ(std::vector<uint8_t>({9, 10, 11, 12}), segment3.Lookup(0, 4));
  }
}

TEST_F(LogTest, LookupEof) {
  Log<InMemorySegment> log{GeneratePath(), 3};
  log.Append({1, 2, 3, 4});
  EXPECT_TRUE(log.Lookup(200U, 3U).empty());
}

}  // namespace wombat::log::testing
