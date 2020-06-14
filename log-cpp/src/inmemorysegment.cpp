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

// TODO use memfd_create - then should be able to share logic between
// system segement and this (only 'open' changes)

// Store in-memory segment state in a singeton to act like a filesystem.
class InMemorySegmentState {
 public:
  InMemorySegmentState() {}

  InMemorySegmentState(const InMemorySegmentState&) = delete;

  void operator=(const InMemorySegmentState&)  = delete;

  static InMemorySegmentState& GetInstance() {
    static InMemorySegmentState instance{};
    return instance;
  }

  std::unordered_map<std::string, std::vector<uint8_t>>& state() {
    return state_;
  }

 private:
  std::unordered_map<std::string, std::vector<uint8_t>> state_;
};

InMemorySegment::InMemorySegment(
    uint64_t id,
    const std::filesystem::path& path,
    size_t limit
) : Segment(limit), path_{path / IdToName(id)} {
  auto& state = InMemorySegmentState().GetInstance().state();
  if (state.find(path_.string()) == state.end()) {
    state.emplace(path_.string(), std::vector<uint8_t>{});
  } else {
    size_ = state.at(path_.string()).size();
  }
}

void InMemorySegment::Append(const std::vector<uint8_t>& data) {
  std::vector<uint8_t>& buf = InMemorySegmentState().GetInstance().state().at(path_.string());
  buf.insert(buf.end(), data.begin(), data.end());
  size_ += data.size();
}

std::vector<uint8_t> InMemorySegment::Lookup(uint64_t offset, uint64_t size) {
  std::vector<uint8_t>& buf = InMemorySegmentState().GetInstance().state().at(path_.string());
  if (buf.size() < offset + size) return {};
  return std::vector(buf.begin() + offset, buf.begin() + offset + size);
};

uint64_t InMemorySegment::Send(uint64_t offset, uint64_t size, int fd) {
  // This is inefficient but used to simulate sendfile of system segment - as
  // this is only for testing the extra copy is fine.
  int in_fd = memfd_create(path_.c_str(), O_RDWR);
  if (in_fd == -1) {
    throw LogException{"memfd_create failed"};
  }

  std::vector<uint8_t>& buf = InMemorySegmentState().GetInstance().state().at(path_.string());
  if (write(in_fd, buf.data(), buf.size()) != (ssize_t) buf.size()) {
    throw LogException{"write to memfd_create failed"};
  }

  off_t off = offset;
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

uint64_t InMemorySegment::Recv(uint64_t size, int fd) {
  std::vector<uint8_t> buf(size);

  if (lseek(fd, 0, SEEK_SET) != 0) return 0;

  ssize_t n = read(fd, buf.data(), buf.size());
  if (n == -1) {
    if (errno == EAGAIN) {
      return 0;
    } else {
      throw LogException{"read failed"};
    }
  }

  Append(std::vector<uint8_t>(buf.begin(), buf.begin() + n));

  return n;
}

}  // namespace wombat::log
