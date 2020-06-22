#include "log/segment.h"

#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <filesystem>

#include <glog/logging.h>
#include "log/logexception.h"

namespace wombat::log {

Segment::Segment(Segment&& segment) {
  path_ = std::move(segment.path_);
  size_ = segment.size_;
  fd_ = segment.fd_;
  // Set moved from fd to negative to avoid closing.
  segment.fd_ = -1;
}

Segment& Segment::operator=(Segment&& segment) {
  path_ = std::move(segment.path_);
  size_ = segment.size_;
  fd_ = segment.fd_;
  // Set moved from fd to negative to avoid closing.
  segment.fd_ = -1;

  return *this;
}

void Segment::Append(const std::vector<uint8_t>& data) {
  // Must seek end as O_APPEND cannot be used.
  if (lseek(fd_, 0, SEEK_END) == -1) {
    LOG(ERROR) << "segment append failed - lseek " << strerror(errno);
    throw LogException{"segment lseek error", errno};
  }

  ssize_t size = data.size();
  ssize_t n = 0;
  while (n < size) {
    ssize_t written = write(fd_, data.data(), data.size());
    if (written == -1) {
      throw LogException{"segment write error", errno};
    }
    n += written;
  }
  size_ += data.size();
}

std::vector<uint8_t> Segment::Lookup(uint32_t offset, uint32_t size) {
  if (lseek(fd_, offset, SEEK_SET) == -1) {
    throw LogException{"segment lseek error", errno};
  }
  std::vector<uint8_t> data(size);
  ssize_t remaining = size;
  while (0 < remaining) {
    ssize_t n = read(fd_, data.data(), data.size());
    if (n == -1) {
      throw LogException{"segment read error", errno};
    } else if (n == 0) {  // EOF
      return {};
    }
    remaining -= n;
  }
  return data;
};

uint32_t Segment::Send(uint32_t offset, uint32_t size, int fd) {
  off_t off = offset;
  ssize_t written = sendfile(fd, fd_, &off, size);
  if (written == -1) {
    if (errno == EAGAIN) {
      return 0;
    } else {
      throw LogException{"segment sendfile failed", errno};
    }
  }
  return written;
}

uint32_t Segment::Size() const {
  off_t seek = lseek(fd_, 0, SEEK_END);
  if (seek == -1) {
    throw LogException{"segment lseek failed", errno};
  }
  return seek;
}

Segment::Segment(const std::filesystem::path& path, uint32_t limit)
    : path_{path}, size_{0}, limit_{limit} {}

std::string IdToName(uint32_t id) {
  std::string id_str = std::to_string(id);
  return "segment-" + std::string(ID_PADDING - id_str.length(), '0') + id_str;
}

}  // namespace wombat::log
