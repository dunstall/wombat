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

Connection::~Connection() {
  if (connfd_ >= 0) {
    LOG(INFO) << "closing connection to ...";
    close(connfd_);
  }
}

Connection::Connection(Connection&& conn) {
  connfd_ = conn.connfd_;
  // Set to -1 so the socket is not closed.
  conn.connfd_ = -1;

  addr_ = conn.addr_;
  offset_ = conn.offset_;
  buf_ = std::move(conn.buf_);
  state_ = conn.state_;
}

Connection& Connection::operator=(Connection&& conn) {
  connfd_ = conn.connfd_;
  // Set to -1 so the socket is not closed.
  conn.connfd_ = -1;

  addr_ = conn.addr_;
  offset_ = conn.offset_;
  buf_ = std::move(conn.buf_);
  state_ = conn.state_;
  return *this;
}

// TODO(AD) If read returns false leader must remove from map and fds
bool Connection::Read() {
  // TODO if already established only reading for errors

  int n;
  if ((n = read(connfd_, buf_.data(), kReadBufSize)) == -1) {
    if (errno == ECONNRESET) {
      LOG(WARNING) << "connection reset by replica";
    } else {
      LOG(WARNING) << "error reading from replica " << std::strerror(errno);
    }
    close(connfd_);
    return false;
  } else if (n == 0) {
    LOG(WARNING) << "connection reset by replica";
    close(connfd_);
    return false;
  } else {
    // If already established discard data.
    if (state_ == ConnectionState::kEstablished) {
      return true;
    }

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
