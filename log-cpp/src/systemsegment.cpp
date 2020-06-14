#include "log/systemsegment.h"

#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <filesystem>

#include "log/logexception.h"

namespace wombat::log {

SystemSegment::SystemSegment(
    uint64_t id,
    const std::filesystem::path& path,
    size_t limit)
  : Segment{limit}, path_{path / IdToName(id) }
{
  std::filesystem::create_directories(path);

  fs_.open(path_, std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
  size_ = std::filesystem::file_size(path_);

  if (fs_.fail()) {
    throw LogException{"failed to open file"};
  }
}

void SystemSegment::Append(const std::vector<uint8_t>& data)
{
  size_ += data.size();
  fs_.write((char*) data.data(), data.size());
  if (fs_.fail() || fs_.bad()) {
    throw LogException{"failed to write to file"};
  }
  if (fs_.sync() == -1) {  // TODO(AD) only on timeout or size reached?
    throw LogException{"failed to sync file"};
  }
}

std::vector<uint8_t> SystemSegment::Lookup(uint64_t offset, uint64_t size)
{
  fs_.seekp(offset);
  if (fs_.eof()) {
    return {};
  } else if (fs_.fail() || fs_.bad()) {
    throw LogException{"failed to seek file"};
  }

  std::vector<uint8_t> data(size);
  fs_.read((char*) data.data(), size);
  if (fs_.eof()) {
    return {};
  } else if (fs_.fail() || fs_.bad()) {
    throw LogException{"failed to read file"};
  }

  return data;
}

uint64_t SystemSegment::Send(uint64_t offset, uint64_t size, int fd) {
  // TODO(AD) should not have to open every time - just use FILE* for all?
  int in_fd = open(path_.c_str(), O_RDONLY);
  if (in_fd == -1) {
    throw LogException{"failed to open segment to send"};
  }

  off_t off = offset;
  //TODO(AD) first send header - TCP_CORK (see sendfile docs)
  ssize_t written = sendfile(fd, in_fd, &off, size);
  if (written == -1) {
    if (errno == EAGAIN) {
      return 0;
    } else {
      throw LogException{"sendfile failed"};
    }
  }

  return written;
}

uint64_t SystemSegment::Recv(uint64_t size, int fd) {
  int segment_fd = open(path_.c_str(), O_WRONLY);
  if (segment_fd == -1) {
    throw LogException{"failed to open segment to recv"};
  }

  off_t off = 0;
  ssize_t written = sendfile(segment_fd, fd, &off, size);
  if (written == -1) {
    if (errno == EAGAIN) {
      return 0;
    } else {
      throw LogException{"sendfile failed"};
    }
  }

  size_ += written;

  return written;
}

}  // namespace wombat::log
