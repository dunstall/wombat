#include <vector>

#include "gtest/gtest.h"
#include "log/inmemorysegment.h"
#include "log/log.h"
#include "log/partition.h"
#include "tempdir.h"

namespace wombat::log::testing {

class PartitionTest : public ::testing::Test {};

TEST_F(PartitionTest, OpenEmpty) {
  Log<InMemorySegment> log{GeneratePath(), 3};
  Partition partition{log};

  std::vector<uint8_t> read;
  uint32_t next;
  EXPECT_FALSE(partition.Lookup(0U, &read, &next));
}

TEST_F(PartitionTest, LookupOffsetZero) {
  Log<InMemorySegment> log{GeneratePath(), 3};
  Partition partition{log};

  const std::vector<uint8_t> data{1, 2, 3, 4, 5, 6};
  partition.Append(data);

  std::vector<uint8_t> read;
  uint32_t next;

  EXPECT_TRUE(partition.Lookup(0, &read, &next));
  EXPECT_EQ(data, read);
  EXPECT_EQ(10U, next);
}

TEST_F(PartitionTest, LookupMulti) {
  Log<InMemorySegment> log{GeneratePath(), 3};
  Partition partition{log};

  const std::vector<uint8_t> data1{1, 2, 3};
  partition.Append(data1);
  const std::vector<uint8_t> data2{0xa, 0xb, 0xc};
  partition.Append(data2);
  const std::vector<uint8_t> data3{0xff, 0xff, 0xff};
  partition.Append(data3);

  std::vector<uint8_t> read;
  uint32_t next;

  EXPECT_TRUE(partition.Lookup(0, &read, &next));
  EXPECT_EQ(data1, read);
  EXPECT_EQ(7U, next);

  EXPECT_TRUE(partition.Lookup(next, &read, &next));
  EXPECT_EQ(data2, read);
  EXPECT_EQ(14U, next);

  EXPECT_TRUE(partition.Lookup(next, &read, &next));
  EXPECT_EQ(data3, read);
  EXPECT_EQ(21U, next);
}

}  // namespace wombat::log::testing
