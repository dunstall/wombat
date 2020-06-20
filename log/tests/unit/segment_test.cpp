#include "log/segment.h"
#include "gtest/gtest.h"

namespace wombat::log::testing {

TEST(IdToName, IdToName) {
  EXPECT_EQ("segment-00000000000000000000", IdToName(0));
  EXPECT_EQ("segment-00000000000000078642", IdToName(78642));
  EXPECT_EQ("segment-18446744073709551615", IdToName(0xffffffffffffffff));
}

}  // namespace wombat::log::testing
