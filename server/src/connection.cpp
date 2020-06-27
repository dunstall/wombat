#include "server/connection.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

#include <cstring>
#include <string>
#include <vector>

#include <glog/logging.h>

namespace wombat::log {

Connection::Connection(int connfd, const struct sockaddr_in& addr)
    : connfd_{connfd}, buf_(kReadBufSize) {
  address_ = AddrToString(addr);
  LOG(INFO) << "accepted pending connection to " << address();
}

Connection::~Connection() {
  if (connfd_ >= 0) {
    LOG(INFO) << "closing connection to " << address();
    close(connfd_);
  }
}

Connection::Connection(Connection&& conn) {
  connfd_ = conn.connfd_;
  // Set to -1 so the socket is not closed.
  conn.connfd_ = -1;

  buf_ = std::move(conn.buf_);
  address_ = std::move(conn.address_);
}

Connection& Connection::operator=(Connection&& conn) {
  connfd_ = conn.connfd_;
  // Set to -1 so the socket is not closed.
  conn.connfd_ = -1;

  buf_ = std::move(conn.buf_);
  address_ = std::move(conn.address_);
  return *this;
}

bool Connection::Read() {
  int n;
  if ((n = read(connfd_, buf_.data(), kReadBufSize)) == -1) {
    if (errno == ECONNRESET) {
      LOG(WARNING) << "connection reset by client";
    } else {
      LOG(WARNING) << "error reading from client " << std::strerror(errno);
    }
    close(connfd_);
    return false;
  } else if (n == 0) {
    LOG(WARNING) << "connection reset by client";
    close(connfd_);
    return false;
  } else {
    // TODO(AD) Read requests

    // // TODO(AD) Incramental read
    // if (n == kReadBufSize) {
      // uint32_t offset = 0;
      // memcpy(&offset, buf_.data(), kReadBufSize);
      // offset_ = ntohl(offset);
      // LOG(INFO) << "connection established to " << address_ << " offset: " << offset_;
      // state_ = ConnectionState::kEstablished;
    // }

    return true;
  }
  return false;
}

std::string Connection::AddrToString(const struct sockaddr_in& addr) const {
  std::string s(INET_ADDRSTRLEN, '\0');
  inet_ntop(AF_INET, &addr.sin_addr.s_addr, (char*) s.c_str(), INET_ADDRSTRLEN);
  return s + ":" + std::to_string(ntohs(addr.sin_port));
}

}  // namespace wombat::log
