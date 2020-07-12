// Copyright 2020 Andrew Dunstall

#include "connection/streamsocket.h"

#include <cstdint>
#include <string>
#include <vector>

namespace wombat::broker::connection {

StreamSocket::~StreamSocket() {
  // TODO(AD) Close socket
}

StreamSocket::StreamSocket(StreamSocket&& sock) {
  // TODO(AD) Ensure socket doesnt close
}

StreamSocket& StreamSocket::operator=(StreamSocket&& sock) {
  // TODO(AD) Ensure socket doesnt close
  return *this;
}

void StreamSocket::Connect(const std::string& ip, uint16_t port) {
  // TODO(AD) Connect existing socket
}

size_t StreamSocket::Read(std::vector<uint8_t>* buf, size_t from, size_t n) {
  // TODO(AD) Read into buf->data() + from
  return 0;
}

size_t StreamSocket::Write(const std::vector<uint8_t>& buf, size_t from) {
  // TODO(AD) Write from buf->data() + from
  return 0;
}

}  // namespace wombat::broker::connection
