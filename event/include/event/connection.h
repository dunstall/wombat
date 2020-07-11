// Copyright 2020 Andrew Dunstall

#pragma once

#include <netinet/in.h>
#include <sys/socket.h>

#include <optional>
#include <string>
#include <vector>

#include "frame/message.h"

namespace wombat::broker {

class Connection {
 public:
  Connection() = default;
  Connection(int connfd, const struct sockaddr_in& addr);

  virtual ~Connection();

  Connection(const Connection& conn) = delete;
  Connection& operator=(const Connection& conn) = delete;

  Connection(Connection&& conn) = default;
  Connection& operator=(Connection&& conn) = default;

  int connfd() const { return connfd_; }

  std::string address() const { return address_; }

  virtual std::optional<record::Message> Receive() = 0;

  virtual bool Send(const record::Message& msg) = 0;

 protected:
  int connfd_;

  std::string address_;
};

}  // namespace wombat::broker
