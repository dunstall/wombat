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
#include "record/message.h"

namespace wombat::broker::server {

Connection::Connection(int connfd, const struct sockaddr_in& addr)
    : connfd_{connfd},
      incoming_buf_(kReadBufSize),
      state_(State::kHeaderPending),
      request_bytes_remaining_{record::MessageHeader::kSize} {
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

  outgoing_buf_ = std::move(conn.outgoing_buf_);
  incoming_buf_ = std::move(conn.incoming_buf_);
  address_ = std::move(conn.address_);
}

Connection& Connection::operator=(Connection&& conn) {
  connfd_ = conn.connfd_;
  // Set to -1 so the socket is not closed.
  conn.connfd_ = -1;

  outgoing_buf_ = std::move(conn.outgoing_buf_);
  address_ = std::move(conn.address_);
  return *this;
}

std::optional<record::Message> Connection::Receive() {
  int n = read(
      connfd_, incoming_buf_.data() + n_read_, request_bytes_remaining_
  );
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

bool Connection::Send(const std::vector<uint8_t> data) {
  // TODO(AD) Use outgoing_buf_. For now just assume all data can be written.
  return (write(connfd_, data.data(), data.size())
      == static_cast<int>(data.size()));
}

std::optional<record::Message> Connection::HandleRead(int n) {
  request_bytes_remaining_ -= n;
  n_read_ += n;

  if (state_ == State::kHeaderPending && request_bytes_remaining_ == 0) {
    header_ = record::MessageHeader::Decode(incoming_buf_);
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
    request_bytes_remaining_ = record::MessageHeader::kSize;
    n_read_ = 0;
    return record::Message{
        header_->type(),
        std::vector<uint8_t>(
            incoming_buf_.begin(),
            incoming_buf_.begin() + header_->payload_size()
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
