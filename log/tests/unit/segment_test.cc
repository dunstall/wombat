// Copyright 2020 Andrew Dunstall

#include "log/segment.h"
#include "gtest/gtest.h"

namespace wombat::broker::log::testing {

TEST(IdToName, IdToName) {
  EXPECT_EQ("segment-00000000000000000000", IdToName(0));
  EXPECT_EQ("segment-00000000000000078642", IdToName(78642));
  EXPECT_EQ("segment-00000000004294967295", IdToName(0xffffffff));
}

}  // namespace wombat::broker::log::testing
