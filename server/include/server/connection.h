// Copyright 2020 Andrew Dunstall

#pragma once

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

namespace wombat::broker::server {

// Represents connection from leader to replica
template<class R>
class Connection {
 public:
  Connection(int connfd, const struct sockaddr_in& addr)
      : connfd_{connfd}, buf_(kReadBufSize), received_{} {
    address_ = AddrToString(addr);
    LOG(INFO) << "accepted pending connection to " << address();
  }

  ~Connection() {
    if (connfd_ >= 0) {
      LOG(INFO) << "closing connection to " << address();
      close(connfd_);
    }
  }

  Connection(const Connection& conn) = delete;
  Connection& operator=(const Connection& conn) = delete;

  Connection(Connection&& conn) {
    connfd_ = conn.connfd_;
    // Set to -1 so the socket is not closed.
    conn.connfd_ = -1;

    buf_ = std::move(conn.buf_);
    address_ = std::move(conn.address_);
  }

  Connection& operator=(Connection&& conn) {
    connfd_ = conn.connfd_;
    // Set to -1 so the socket is not closed.
    conn.connfd_ = -1;

    buf_ = std::move(conn.buf_);
    address_ = std::move(conn.address_);
    return *this;
  }

  std::string address() const { return address_; }

  // TODO(AD) Cleanup interface
  // Send(T) - need something that manages polling connections for writing

  bool Read() {
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

      const std::optional<R> record
          = R::Decode(buf_);
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

  bool Write();

  std::vector<R> Received() {
    std::vector<R> recv = std::move(received_);
    received_ = std::vector<R>{};
    return recv;
  }

  bool Send();  // Push response to buffer

 private:
  static const size_t kReadBufSize = 1024;  // TODO(AD)

  std::string AddrToString(const struct sockaddr_in& addr) const {
    std::string s(INET_ADDRSTRLEN, '\0');
    inet_ntop(
        AF_INET,
        &addr.sin_addr.s_addr,
        const_cast<char*>(s.c_str()),
        INET_ADDRSTRLEN
    );
    return s + ":" + std::to_string(ntohs(addr.sin_port));
  }

  int connfd_;

  std::vector<uint8_t> buf_;

  std::string address_;

  std::vector<R> received_;
};

}  // namespace wombat::broker::server
