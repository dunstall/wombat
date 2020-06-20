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
    : connfd_{connfd}, addr_{addr}, buf_(kReadBufSize), state_{ConnectionState::kPending} {}

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
      // // // TODO(AD) Expect sockfd to be in pending connections (warn if not) - 
      // // // try to read until buffer full then decode offset and create Connection
      // // // to connections_

      // // TODO if 4 then read offset

    LOG(INFO) << "received " << n << " bytes";

    // TODO keep reading till buf

    if (n == kReadBufSize) {
      LOG(INFO) << buf_.size();
      for (uint8_t b : buf_) {
        LOG(INFO) << "BYTE " << (int)b;
      }

      uint32_t offset = 0;
      memcpy(&offset, buf_.data(), kReadBufSize);
      LOG(INFO) << "temp offset " << offset;
      offset_ = ntohl(offset);
      LOG(INFO) << "connection established: offset: " << offset_ << std::endl;
      state_ = ConnectionState::kEstablished;
    }

    return true;
  }
  return false;
}

}  // namespace wombat::log
