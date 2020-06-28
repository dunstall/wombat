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
#include "record/request.h"

namespace wombat::broker::server {

class Connection {
 public:
  Connection(int connfd, const struct sockaddr_in& addr);

  ~Connection();

  Connection(const Connection& conn) = delete;
  Connection& operator=(const Connection& conn) = delete;

  Connection(Connection&& conn);

  Connection& operator=(Connection&& conn);

  std::string address() const { return address_; }

  // TODO(AD) Cleanup interface
  // Send(T) - need something that manages polling connections for writing

  bool Read();

  bool Write();

  std::vector<record::Request> Received();

  bool Send();  // Push response to buffer

 private:
  static const size_t kReadBufSize = 1024;  // TODO(AD)

  std::string AddrToString(const struct sockaddr_in& addr) const;

  int connfd_;

  std::vector<uint8_t> buf_;

  std::string address_;

  std::vector<record::Request> received_;
};

}  // namespace wombat::broker::server
