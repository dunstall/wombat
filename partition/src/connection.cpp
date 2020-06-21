#include "partition/connection.h"

#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

#include <cstring>
#include <vector>

#include <glog/logging.h>

namespace wombat::log {

Connection::Connection(int connfd, const struct sockaddr_in& addr)
    : connfd_{connfd}, addr_{addr}, buf_(kReadBufSize), state_{ConnectionState::kPending} {
  LOG(INFO) << "accepted pending connection to...";  // TODO(ADDR)
  offset_ = 0;
}

// TODO(AD) If read returns false leader must remove from map and fds
bool Connection::Read() {
  int n;
  // if ((n = read(connfd_, buf_.data() + (kReadBufSize - remaining_), remaining_)) == -1) {
  if ((n = read(connfd_, buf_.data(), kReadBufSize)) == -1) {
    if (errno == ECONNRESET) {
      LOG(WARNING) << "connection reset by replica";
    } else {
      LOG(WARNING) << "error reading from replica " << std::strerror(errno);
    }
    close(connfd_);
    return false;
  } else if (n == 0) {
    close(connfd_);
    return false;
  } else {
    // TODO(AD) Incramental read
    if (n == kReadBufSize) {
      uint32_t offset = 0;
      memcpy(&offset, buf_.data(), kReadBufSize);
      offset_ = ntohl(offset);
      LOG(INFO) << "connection established: offset: " << offset_ << std::endl;  // TODO(AD) Log addr
      state_ = ConnectionState::kEstablished;
    }

    return true;
  }
  return false;
}

}  // namespace wombat::log
