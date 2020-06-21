#pragma once

#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

#include <cstdint>
#include <string>
#include <vector>

namespace wombat::log {

enum class ConnectionState {
  // Pending indicates the TCP connection has been setup but the handshake
  // is incomplete.
  kPending,
  kEstablished
};

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

  uint32_t offset() const { return offset_; }

  void set_offset(uint32_t offset) { offset_ = offset; }

  ConnectionState state() const { return state_; }

  // TODO(AD) Attempt to read the offset - buffer incase < 4 bytes read.
  bool Read();

 private:
  static const size_t kReadBufSize = 4;

  std::string AddrToString(const struct sockaddr_in& addr) const;

  int connfd_;

  uint32_t offset_;

  std::vector<uint8_t> buf_;

  ConnectionState state_;

  std::string address_;
};

}  // namespace wombat::log
