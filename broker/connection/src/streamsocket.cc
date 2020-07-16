// Copyright 2020 Andrew Dunstall

#include "connection/streamsocket.h"

#include <errno.h>
#include <unistd.h>

#include <cstdint>
#include <string>
#include <vector>

#include "connection/connectionexception.h"

namespace wombat::broker::connection {

StreamSocket::StreamSocket(int sockfd) : sockfd_{sockfd} {}

StreamSocket::~StreamSocket() {
  if (sockfd_) {
    close(*sockfd_);
  }
}

StreamSocket::StreamSocket(StreamSocket&& sock) {
  sockfd_ = sock.sockfd_;
  // Set sock.sockfd so it does not close when sock destructs.
  sock.sockfd_ = std::nullopt;
}

StreamSocket& StreamSocket::operator=(StreamSocket&& sock) {
  sockfd_ = sock.sockfd_;
  // Set sock.sockfd so it does not close when sock destructs.
  sock.sockfd_ = std::nullopt;
  return *this;
}

void StreamSocket::Connect(const std::string& ip, uint16_t port) {
  // TODO(AD) Connect existing socket
  // TODO(AD) Non-blocking
}

size_t StreamSocket::Read(std::vector<uint8_t>* buf, size_t from, size_t n) {
  if (!sockfd_) {
    throw ConnectionException{"reading from closed connection"};
  }

  ssize_t n_read = read(*sockfd_, buf->data() + from, n);
  if (n_read < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return 0;
    } else {
      throw ConnectionException{"error reading from socket", errno};
    }
  } else if (n_read == 0) {
    // TODO(AD) Incorrect use of exceptions - this isn't exceptional.
    throw ConnectionException{"connection closed by client"};
  }
  return n_read;
}

size_t StreamSocket::Write(const std::vector<uint8_t>& buf, size_t from,
                           size_t n) {
  if (!sockfd_) {
    throw ConnectionException{"writing to closed connection"};
  }

  ssize_t n_written = write(*sockfd_, buf.data() + from, n);
  if (n_written < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return 0;
    } else {
      throw ConnectionException{"error writing to socket", errno};
    }
  }
  return n_written;
}

}  // namespace wombat::broker::connection
