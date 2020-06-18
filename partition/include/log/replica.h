#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <vector>

#include "log/log.h"
#include "log/logexception.h"

namespace wombat::log {

struct LeaderAddress {
  std::string ip;
  uint16_t port;
};

template<class S>
class Replica {
 public:
  Replica(Log<S> log, const LeaderAddress& leader)
      : log_{log}, leader_{leader}, buf_(kBufSize), connected_{false}
  {
    signal(SIGINT, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
  }

  ~Replica() {
    close(sock_);
  }

  Replica(const Replica&) = delete;
  Replica& operator=(const Replica&) = delete;

  Replica(Replica&&) = delete;
  Replica& operator=(Replica&&) = delete;

  void Poll() {
    if (!connected_) {
      if (!Connect()) {
        return;
      }
    }

    ssize_t n = read(sock_, buf_.data(), kBufSize);
    if (n == 0) {
      connected_ = false;
      return;
    } else if (n == -1) {
      if (errno != EAGAIN || errno != EWOULDBLOCK) {
        throw LogException{"read error"};
      } else {
        return;
      }
    }
    log_.Append(std::vector<uint8_t>(buf_.begin(), buf_.begin() + n));
  }

 private:
  static const size_t kBufSize = 1024;

  bool Connect() {
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(leader_.port);
    if (inet_pton(AF_INET, leader_.ip.c_str(), &servaddr.sin_addr.s_addr) != 1) {
      throw LogException{"inet_pton error"};
    }

    if ((sock_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      throw LogException{"socket error"};
    }

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout)) == -1)
      throw LogException{"setsockopt error"};

    if (setsockopt(sock_, SOL_SOCKET, SO_SNDTIMEO, (char*) &timeout, sizeof(timeout)) == -1) {
      throw LogException{"setsockopt error"};
    }

    if (connect(sock_, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
      throw LogException{"failed to connect to server"};
    }

    if (!SendOffset()) return false;

    connected_ = true;

    return true;
  }

  bool SendOffset() {
    uint32_t offset = log_.size();
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
          throw LogException{"failed to write to server"};
        }
      }
      written += n;
    }

    return true;
  }

  Log<S> log_;

  LeaderAddress leader_;

  int sock_;

  std::vector<uint8_t> buf_;

  bool connected_;
};

}  // namespace wombat::log
