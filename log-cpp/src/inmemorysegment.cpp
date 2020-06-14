#include "log/inmemorysegment.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <unistd.h>

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

#include "log/logexception.h"
#include "log/segment.h"

namespace wombat::log {

// TODO refactor to share code with systemsegment

// Store in-memory segment state (file descripor) in a singeton to act like a
// persistent filesystem.
class InMemorySegmentState {
 public:
  InMemorySegmentState() {}

  InMemorySegmentState(const InMemorySegmentState&) = delete;

  static InMemorySegmentState& GetInstance() {
    static InMemorySegmentState instance{};
    return instance;
  }

  std::unordered_map<std::string, int>& state() {
    return state_;
  }

 private:
  std::unordered_map<std::string, int> state_;
};

InMemorySegment::InMemorySegment(
    uint64_t id,
    const std::filesystem::path& path,
    size_t limit
) : Segment(limit), path_{path / IdToName(id)} {
  auto& state = InMemorySegmentState().GetInstance().state();
  if (state.find(path_.string()) == state.end()) {
    fd_ = memfd_create(path_.c_str(), O_RDWR);
    if (fd_ == -1) {
      throw LogException{"memfd_create failed"};
    }
    state.emplace(path_.string(), fd_);
  } else {
    fd_ = state.at(path_.string());
  }

  off_t seek = lseek(fd_, 0, SEEK_END);
  if (seek == -1) {
    throw LogException{"lseek failed"};  // TODO add errno to exception
  }
  size_ = seek;
}

InMemorySegment::~InMemorySegment() {
  // Do not close fd_ as this would destroy the in-memory state. This is
  // ok as only for testing.
}

void InMemorySegment::Append(const std::vector<uint8_t>& data) {
  ssize_t size = data.size();
  ssize_t n = 0;
  while (n < size) {
    ssize_t written = write(fd_, data.data(), data.size());
    if (written == -1) {
      throw LogException{"write error"};
    }
    n += written;
  }
  size_ += data.size();
}

std::vector<uint8_t> InMemorySegment::Lookup(uint64_t offset, uint64_t size) {
  if (lseek(fd_, offset, SEEK_SET) == -1) {
    throw LogException{"lseek error"};
  }
  std::vector<uint8_t> data(size);
  ssize_t remaining = size;
  while (0 < remaining) {
    ssize_t n = read(fd_, data.data(), data.size());
    if (n == -1) {
      throw LogException{"read error"};
    } else if (n == 0) {  // EOF
      return {};
    }
    remaining -= n;
  }
  return data;
};

uint64_t InMemorySegment::Send(uint64_t offset, uint64_t size, int fd) {
  off_t off = offset;
  ssize_t written = sendfile(fd, fd_, &off, size);
  if (written == -1) {
    if (errno == EAGAIN) {
      return 0;
    } else {
      throw LogException{"sendfile failed"};
    }
  }
  return written;
}

uint64_t InMemorySegment::Recv(uint64_t size, int fd) {
  off_t off = 0;
  ssize_t written = sendfile(fd_, fd, &off, size);
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
