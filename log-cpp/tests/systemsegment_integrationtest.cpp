#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>
#include <random>
#include <string>

#include "gtest/gtest.h"
#include "log/systemsegment.h"
#include "tempdir.h"

namespace wombat::log::testing {

class SystemSegmentTest : public ::testing::Test {};

TEST_F(SystemSegmentTest, OpenEmpty) {
  TempDir dir{};
  SystemSegment segment{0x2478, dir.path(), 3};

  EXPECT_EQ(0U, segment.size());
  EXPECT_FALSE(segment.is_full());

  const std::vector<uint8_t> data{1, 2, 3};
  segment.Append(data);

  EXPECT_EQ(3U, segment.size());
  EXPECT_TRUE(segment.is_full());

  EXPECT_EQ(data, segment.Lookup(0U, 3U));
}

TEST_F(SystemSegmentTest, OpenDirNotExist) {
  TempDir dir{};
  SystemSegment segment{0x2478, dir.path() / "notexist", 3};

  EXPECT_EQ(0U, segment.size());
  EXPECT_FALSE(segment.is_full());

  const std::vector<uint8_t> data{1, 2, 3};
  segment.Append(data);

  EXPECT_EQ(3U, segment.size());
  EXPECT_TRUE(segment.is_full());

  EXPECT_EQ(data, segment.Lookup(0U, 3U));
}

TEST_F(SystemSegmentTest, OpenExisting) {
  TempDir dir{};
  const std::string path = dir.path();

  SystemSegment segment_writter{0x2478, path, 3};

  EXPECT_EQ(0U, segment_writter.size());
  EXPECT_FALSE(segment_writter.is_full());

  const std::vector<uint8_t> data{1, 2, 3};
  segment_writter.Append(data);

  SystemSegment segment_reader{0x2478, path, 3};
  EXPECT_EQ(3U, segment_reader.size());
  EXPECT_TRUE(segment_reader.is_full());

  EXPECT_EQ(data, segment_reader.Lookup(0U, 3U));
}

TEST_F(SystemSegmentTest, OpenMulti) {
  TempDir dir{};
  const std::string path = dir.path();

  SystemSegment segment1{0x2478, path, 3};
  SystemSegment segment2{0x2479, path, 3};
  SystemSegment segment3{0x247A, path, 3};

  const std::vector<uint8_t> data{1, 2, 3};
  segment1.Append(data);

  EXPECT_EQ(3U, segment1.size());
  EXPECT_TRUE(segment1.is_full());
  EXPECT_EQ(data, segment1.Lookup(0U, 3U));

  EXPECT_EQ(0U, segment2.size());
  EXPECT_FALSE(segment2.is_full());
  EXPECT_TRUE(segment2.Lookup(0U, 3U).empty());

  EXPECT_EQ(0U, segment3.size());
  EXPECT_FALSE(segment3.is_full());
  EXPECT_TRUE(segment3.Lookup(0U, 3U).empty());
}

TEST_F(SystemSegmentTest, LookupEof) {
  TempDir dir{};
  SystemSegment segment{0x2478, dir.path(), 3};

  EXPECT_TRUE(segment.Lookup(0U, 3U).empty());
}

}  // namespace wombat::log::testing
