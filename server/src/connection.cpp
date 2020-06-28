// Copyright 2020 Andrew Dunstall

#include "server/connection.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "glog/logging.h"
#include "record/request.h"

namespace wombat::broker::server {

Connection::Connection(int connfd, const struct sockaddr_in& addr)
    : connfd_{connfd},
      buf_(kReadBufSize),
      request_bytes_remaining_{record::RequestHeader::kSize} {
  address_ = AddrToString(addr);
  LOG(INFO) << "accepted connection to " << address();
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

std::optional<record::Request> Connection::Receive() {
  int n = read(connfd_, buf_.data() + n_read_, request_bytes_remaining_);
  if (n < 1) {
    if ((n == -1 && errno == ECONNRESET) || n == 0) {
      LOG(WARNING) << "connection reset by client";
    } else {
      LOG(WARNING) << "error reading from client " << std::strerror(errno);
    }
    throw ConnectionException{};
  }

  return HandleRead(n);
}

std::optional<record::Request> Connection::HandleRead(int n) {
  request_bytes_remaining_ -= n;
  n_read_ += n;

  if (state_ == State::kHeaderPending && request_bytes_remaining_ == 0) {
    header_ = record::RequestHeader::Decode(buf_);
    if (!header_) {
      LOG(WARNING) << "received invalid request header (" << address() << ")";
      throw ConnectionException{};
    }

    state_ = State::kPayloadPending;
    request_bytes_remaining_ = header_->payload_size();
    n_read_ = 0;
  } else if (state_ == State::kPayloadPending
      && request_bytes_remaining_ == 0) {
    state_ = State::kHeaderPending;
    request_bytes_remaining_ = record::RequestHeader::kSize;
    n_read_ = 0;
    return record::Request{
        header_->type(),
        std::vector<uint8_t>(
            buf_.begin(), buf_.begin() + header_->payload_size()
        )
    };
  }

  return std::nullopt;
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

}  // namespace wombat::broker::server
