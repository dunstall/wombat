#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>
#include <random>
#include <string>

#include "gtest/gtest.h"
#include "log/systemsegment.h"

namespace wombat::log::testing {

class TempDir {
 public:
  TempDir() : path_(GeneratePath()) {
    std::filesystem::create_directories(path_);
  }

  ~TempDir() {
    std::filesystem::remove_all(path_);
  }

  std::filesystem::path path() const { return path_; }

 private:
  const size_t PATH_LEN = 6;
  const std::string CHARS = "0123456789abcdefghijklmnopqrstuvwxyz";

  std::filesystem::path GeneratePath() const {
    thread_local static std::mt19937 rg{std::random_device{}()};
    thread_local static std::uniform_int_distribution<std::string::size_type> pick(
      0, CHARS.size() - 2
    );
    std::string dir;
    dir.reserve(PATH_LEN);
    for (size_t i = 0; i != PATH_LEN; ++i) {
      dir += CHARS[pick(rg)];
    }
    return "/tmp/wombatlog" + dir;
  }

  std::filesystem::path path_;
};

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
