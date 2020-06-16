#include <vector>

#include "gtest/gtest.h"
#include "log/inmemorysegment.h"
#include "log/leader.h"
#include "log/log.h"
#include "log/tempdir.h"

namespace wombat::log::testing {

class LeaderTest : public ::testing::Test {};

TEST_F(LeaderTest, OpenEmpty) {
  Leader leader{Log<InMemorySegment>{GeneratePath(), 3}};

  std::vector<uint8_t> read;
  uint32_t next;
  EXPECT_FALSE(leader.Lookup(0U, &read, &next));
}

TEST_F(LeaderTest, LookupOffsetZero) {
  Leader leader{Log<InMemorySegment>{GeneratePath(), 3}};

  const std::vector<uint8_t> data{1, 2, 3, 4, 5, 6};
  leader.Append(data);

  std::vector<uint8_t> read;
  uint32_t next;

  EXPECT_TRUE(leader.Lookup(0, &read, &next));
  EXPECT_EQ(data, read);
  EXPECT_EQ(10U, next);
}

TEST_F(LeaderTest, LookupMulti) {
  Leader leader{Log<InMemorySegment>{GeneratePath(), 3}};

  const std::vector<uint8_t> data1{1, 2, 3};
  leader.Append(data1);
  const std::vector<uint8_t> data2{0xa, 0xb, 0xc};
  leader.Append(data2);
  const std::vector<uint8_t> data3{0xff, 0xff, 0xff};
  leader.Append(data3);

  std::vector<uint8_t> read;
  uint32_t next;

  EXPECT_TRUE(leader.Lookup(0, &read, &next));
  EXPECT_EQ(data1, read);
  EXPECT_EQ(7U, next);

  EXPECT_TRUE(leader.Lookup(next, &read, &next));
  EXPECT_EQ(data2, read);
  EXPECT_EQ(14U, next);

  EXPECT_TRUE(leader.Lookup(next, &read, &next));
  EXPECT_EQ(data3, read);
  EXPECT_EQ(21U, next);
}

}  // namespace wombat::log::testing
