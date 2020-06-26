#pragma once

#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

#include <cstdint>
#include <string>
#include <vector>

namespace wombat::log {

// Represents connection from leader to replica
class Connection {
 public:
  Connection(int connfd, const struct sockaddr_in& addr);

  ~Connection();

  Connection(const Connection& conn) = delete;
  Connection& operator=(const Connection& conn) = delete;

  Connection(Connection&& conn);
  Connection& operator=(Connection&& conn);

  std::string address() const { return address_; }

  bool Read();

  bool Write();

  bool Send();  // Push response to buffer

 private:
  static const size_t kReadBufSize = 1024;  // TODO(AD)

  std::string AddrToString(const struct sockaddr_in& addr) const;

  int connfd_;

  std::vector<uint8_t> buf_;

  std::string address_;
};

}  // namespace wombat::log
