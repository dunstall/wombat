// Copyright 2020 Andrew Dunstall

#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "glog/logging.h"
#include "log/log.h"
#include "log/logexception.h"

namespace wombat::broker {

struct LeaderAddress {
  std::string ip;
  uint16_t port;
};

template<class S>
class Replica {
 public:
  Replica(std::shared_ptr<Log<S>> log, const LeaderAddress& leader)
      : log_{log}, leader_{leader}, buf_(kBufSize), connected_{false}
  {
    signal(SIGPIPE, SIG_IGN);
  }

  ~Replica() {
    if (sock_ >= 0) {
      LOG(INFO) << "replica closing connection";
      close(sock_);
    }
  }

  Replica(const Replica&) = delete;
  Replica& operator=(const Replica&) = delete;

  Replica(Replica&& replica) {
    log_ = replica.log_;
    leader_ = replica.leader_;
    sock_ = replica.sock_;
    // Set to -1 so the connection is not closed.
    replica.sock_ = -1;
    buf_ = std::move(replica.buf_);
    connected_ = replica.connected_;
  }

  Replica& operator=(Replica&& replica) {
    log_ = replica.log_;
    leader_ = replica.leader_;
    sock_ = replica.sock_;
    // Set to -1 so the connection is not closed.
    replica.sock_ = -1;
    buf_ = std::move(replica.buf_);
    connected_ = replica.connected_;

    return *this;
  }

  // TODO(AD) Should be able to split more out here for unit testing

  bool connected() const { return connected_; }

  void Poll() {
    if (!connected_) {
      if (!Connect()) {
        return;
      }
    }

    ssize_t n = read(sock_, buf_.data(), kBufSize);
    LOG(INFO) << "replica read " << n;
    if (n == 0) {
      LOG(WARNING) << "connection closed by leader "
          << leader_.ip << ":" << leader_.port;
      connected_ = false;
      return;
    } else if (n == -1) {
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        close(sock_);
        if (errno == ECONNRESET) {
          connected_ = false;
          return;
        }

        throw LogException{"replica read error", errno};
      } else {
        return;
      }
    }
    log_->Append(std::vector<uint8_t>(buf_.begin(), buf_.begin() + n));
  }

 private:
  static const size_t kBufSize = 1024;

  bool Connect() {
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(leader_.port);
    int rv = inet_pton(AF_INET, leader_.ip.c_str(), &servaddr.sin_addr.s_addr);
    if (rv != 1) {
      throw LogException{"bad leader IP"};
    }

    if ((sock_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      throw LogException{"socket error", errno};
    }

    // TODO(AD) remove timeout - just use non-blocking sockets (or poll is
    // too slow)
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout)) == -1) {  // NOLINT
      throw LogException{"setsockopt error", errno};
    }
    if (setsockopt(sock_, SOL_SOCKET, SO_SNDTIMEO, (char*) &timeout, sizeof(timeout)) == -1) {  // NOLINT
      throw LogException{"setsockopt error", errno};
    }

    if (connect(sock_, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
      close(sock_);
      LOG(WARNING) << "failed to connect to "
          << leader_.ip << ":" << leader_.port;
      if (errno == ECONNREFUSED) {
        return false;
      }
      throw LogException{"failed to connect to server", errno};
    }

    if (!SendOffset()) return false;

    connected_ = true;

    return true;
  }

  bool SendOffset() {
    uint32_t offset = log_->size();
    LOG(INFO) << "send offset " << offset;

    uint32_t ordered = htonl(offset);
    std::vector<uint8_t> enc {
      (uint8_t) (ordered >> 0),
      (uint8_t) (ordered >> 8),
      (uint8_t) (ordered >> 16),
      (uint8_t) (ordered >> 24)
    };

    int written = 0;
    while (written < 4) {
      int n = write(sock_, enc.data() + written, enc.size() - written);
      if (n == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
          if (errno == EPIPE) {
            connected_ = false;
            return false;
          }
          throw LogException{"failed to write to leader", errno};
        }
      }
      written += n;
    }

    return true;
  }

  std::shared_ptr<Log<S>> log_;

  LeaderAddress leader_;

  int sock_;

  std::vector<uint8_t> buf_;

  bool connected_;
};

}  // namespace wombat::broker
