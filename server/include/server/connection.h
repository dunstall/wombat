// Copyright 2020 Andrew Dunstall

#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <exception>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "glog/logging.h"
#include "record/request.h"

namespace wombat::broker::server {

class ConnectionException : public std::exception {};

class Connection {
 public:
  Connection(int connfd, const struct sockaddr_in& addr);

  ~Connection();

  Connection(const Connection& conn) = delete;
  Connection& operator=(const Connection& conn) = delete;

  Connection(Connection&& conn);

  Connection& operator=(Connection&& conn);

  std::string address() const { return address_; }

  // Returns the request received from the connection or nullopt if no request
  // has been received. If an error occurs or the connection is closed throws
  // a ConnectionException.
  std::optional<record::Request> Receive();

 private:
  enum class State {
    kHeaderPending,
    kPayloadPending
  };

  static const size_t kReadBufSize = 1024;

  std::optional<record::Request> HandleRead(int n);

  std::string AddrToString(const struct sockaddr_in& addr) const;

  int connfd_;

  std::vector<uint8_t> buf_;

  std::string address_;

  State state_;
  int request_bytes_remaining_;  // TODO init to Request::kHeaderSize
  int n_read_ = 0;
  std::optional<record::RequestHeader> header_;
};

}  // namespace wombat::broker::server
