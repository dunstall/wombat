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

#include "event/connection.h"
#include "glog/logging.h"
#include "record/message.h"

namespace wombat::broker::server {

class ConnectionException : public std::exception {};

class TcpConnection : public Connection {
 public:
  TcpConnection(int connfd, const struct sockaddr_in& addr);

  virtual ~TcpConnection();

  TcpConnection(const TcpConnection& conn) = delete;
  TcpConnection& operator=(const TcpConnection& conn) = delete;

  TcpConnection(TcpConnection&& conn);
  TcpConnection& operator=(TcpConnection&& conn);

  std::optional<record::Message> Receive() override;

  bool Send(const std::vector<uint8_t> data) override;

 private:
  enum class State {
    kHeaderPending,
    kPayloadPending
  };

  static const size_t kReadBufSize = 1024;

  std::optional<record::Message> HandleRead(int n);

  std::string AddrToString(const struct sockaddr_in& addr) const;

  std::vector<uint8_t> outgoing_buf_;

  std::vector<uint8_t> incoming_buf_;

  State state_;
  int request_bytes_remaining_;
  int n_read_ = 0;
  std::optional<record::MessageHeader> header_;
};

}  // namespace wombat::broker::server