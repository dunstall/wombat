// Copyright 2020 Andrew Dunstall

#include "server/connection.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

#include <cstring>
#include <optional>
#include <string>
#include <vector>

#include "glog/logging.h"
#include "record/producerecord.h"

namespace wombat::broker::produceserver {

Connection::Connection(int connfd, const struct sockaddr_in& addr)
    : connfd_{connfd}, buf_(kReadBufSize), received_{} {
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

    // TODO(AD) For now expect to read whole request at once.
    // TODO(AD) Handle reading multiple reqeusts and partial requests

    const std::optional<record::ProduceRecord> record
        = record::ProduceRecord::Decode(buf_);
    if (record) {
      LOG(INFO) << "received record from " << address();
      received_.push_back(*record);
    } else {
      LOG(WARNING) << "received invalid record from " << address();
    }

    return true;
  }
  return false;
}

std::vector<record::ProduceRecord> Connection::Received() {
  std::vector<record::ProduceRecord> recv = std::move(received_);
  received_ = std::vector<record::ProduceRecord>{};
  return recv;
}

std::string Connection::AddrToString(const struct sockaddr_in& addr) const {
  std::string s(INET_ADDRSTRLEN, '\0');
  inet_ntop(
      AF_INET,
      &addr.sin_addr.s_addr,
      const_cast<char*>(s.c_str()),
      INET_ADDRSTRLEN
  );
  return s + ":" + std::to_string(ntohs(addr.sin_port));
}

}  // namespace wombat::broker::produceserver
