#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <filesystem>
#include <vector>

#include "gtest/gtest.h"
#include "log/inmemorysegment.h"
#include "log/tempdir.h"

namespace wombat::log::testing {

class InMemorySegmentTest : public ::testing::Test {};

TEST_F(InMemorySegmentTest, OpenEmpty) {
  InMemorySegment segment{0x2478, GeneratePath(), 3};

  EXPECT_EQ(0U, segment.size());
  EXPECT_FALSE(segment.is_full());

  const std::vector<uint8_t> data{1, 2, 3};
  segment.Append(data);

  EXPECT_EQ(3U, segment.size());
  EXPECT_TRUE(segment.is_full());

  EXPECT_EQ(data, segment.Lookup(0U, 3U));
}

TEST_F(InMemorySegmentTest, OpenExisting) {
  const std::filesystem::path path = GeneratePath();
  InMemorySegment segment_writter{0x2478, path, 3};

  EXPECT_EQ(0U, segment_writter.size());
  EXPECT_FALSE(segment_writter.is_full());

  const std::vector<uint8_t> data{1, 2, 3};
  segment_writter.Append(data);

  InMemorySegment segment_reader{0x2478, path, 3};
  EXPECT_EQ(3U, segment_reader.size());
  EXPECT_TRUE(segment_reader.is_full());

  EXPECT_EQ(data, segment_reader.Lookup(0U, 3U));
}

TEST_F(InMemorySegmentTest, OpenMulti) {
  const std::string path = GeneratePath();

  InMemorySegment segment1{0x2478, path, 3};
  InMemorySegment segment2{0x2479, path, 3};
  InMemorySegment segment3{0x247A, path, 3};

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

TEST_F(InMemorySegmentTest, LookupEof) {
  InMemorySegment segment{0x2478, GeneratePath(), 3};
  EXPECT_TRUE(segment.Lookup(0U, 3U).empty());
}

TEST_F(InMemorySegmentTest, Send) {
  InMemorySegment segment{0x2478, GeneratePath(), 3};

  const std::vector<uint8_t> data{1, 2, 3};
  segment.Append(data);

  int fd = memfd_create("myfd", O_RDWR);
  if (fd == -1) FAIL();

  EXPECT_EQ(3U, segment.Send(0, 3, fd));

  if (lseek(fd, 0, SEEK_SET) != 0) FAIL();
  uint8_t buf[3];
  if (read(fd, buf, 3) != 3) FAIL();

  std::vector<uint8_t> read(buf, buf+3);
  EXPECT_EQ(data, read);
}

TEST_F(InMemorySegmentTest, SendOverflow) {
  InMemorySegment segment{0x2478, GeneratePath(), 3};

  const std::vector<uint8_t> data{1, 2, 3};
  segment.Append(data);

  int fd = memfd_create("myfd", O_RDWR);
  if (fd == -1) FAIL();

  // Only return 3 bytes even though requesting 10.
  uint64_t size = 10;
  EXPECT_EQ(3U, segment.Send(0, size, fd));

  if (lseek(fd, 0, SEEK_SET) != 0) FAIL();
  uint8_t buf[3];
  if (read(fd, buf, 3) != 3) FAIL();

  std::vector<uint8_t> read(buf, buf+3);
  EXPECT_EQ(data, read);
}

}  // namespace wombat::log::testing
