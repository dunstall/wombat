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

  int connfd() const { return connfd_; }

  std::string address() const { return address_; }

  // Returns the request received from the connection or nullopt if no request
  // has been received. If an error occurs or the connection is closed throws
  // a ConnectionException.
  std::optional<record::Request> Receive();

  // Adds data to the outgoing buffer (if not empty) and attempts to write
  // to the connection. Returns true if the outgoing buffer was fully sent
  // otherwise false. If an error occurs or the connection is closed throws
  // a ConnectionException.
  bool Send(const std::vector<uint8_t> data);

 private:
  enum class State {
    kHeaderPending,
    kPayloadPending
  };

  static const size_t kReadBufSize = 1024;

  std::optional<record::Request> HandleRead(int n);

  std::string AddrToString(const struct sockaddr_in& addr) const;

  int connfd_;

  std::vector<uint8_t> outgoing_buf_;

  std::vector<uint8_t> incoming_buf_;

  std::string address_;

  State state_;
  int request_bytes_remaining_;
  int n_read_ = 0;
  std::optional<record::RequestHeader> header_;
};

}  // namespace wombat::broker::server
