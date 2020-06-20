#pragma once

#include <netinet/in.h>
#include <netinet/ip.h>

#include <cstdint>
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

  // TODO(AD) Need connfd for sendfile. Must also increment offset after send.
  int connfd() const { return connfd_; }

  uint32_t offset() const { return offset_; }

  void set_offset(uint32_t offset) { offset_ = offset; }

  ConnectionState state() const { return state_; }

  // TODO(AD) Attempt to read the offset - buffer incase < 4 bytes read.
  bool Read();

 private:
  static const size_t kReadBufSize = 4;

  int connfd_;

  struct sockaddr_in addr_;

  uint32_t offset_;

  int remaining_;

  std::vector<uint8_t> buf_;

  ConnectionState state_;
};

}  // namespace wombat::log
