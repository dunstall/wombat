#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

#include "log/log.h"
#include "log/logexception.h"

namespace wombat::log {

struct ReplicaAddress {
  std::string ip;
  uint16_t port;
};

template<class S>
class Leader {
 public:
  Leader(Log<S> log) : log_{log}, buf_(kReadBufSize) {
    running_ = true;
    thread_ = std::thread{&Leader::Run, this};
  }

  ~Leader() {
    running_ = false;
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  Leader(const Leader&) = delete;
  Leader& operator=(const Leader&) = delete;

  Leader(Leader&&) = delete;
  Leader& operator=(Leader&&) = delete;

 private:
  static const uint16_t kListenPort = 3111;
  static const int kListenBacklog = 10;
  static const int kMaxReplicas = 10;
  static const int kPollTimeoutMs = 1000;

  static const size_t kReadBufSize = 1000;

  void Run() {
    int i;
    int maxi = 0;

    Listen();

    struct pollfd client[kMaxReplicas];
    client[0].fd = sock_;
    client[0].events = POLLRDNORM;
    for (i = 1; i < kMaxReplicas; ++i) {
      client[i].fd = -1;
    }

    int nready;
    while (running_) {
      // Need a timeout so periodically check running_.
      if ((nready = poll(client, maxi + 1, kPollTimeoutMs)) == -1) {
        throw LogException{"poll error"};
      }

      if (client[0].revents & POLLRDNORM) {  // New connection.
        struct sockaddr_in cliaddr;
        socklen_t clilen = sizeof(cliaddr);
        
        int connfd = accept(sock_, (struct sockaddr*) &cliaddr, &clilen);
        if (connfd == -1) {
          // TODO See accept(2) for errors to handle otherwise ignore
          // (make this a function and just skip)
        }

        for (i = 1; i != kMaxReplicas; ++i) {
          if (client[i].fd < 0) {
            client[i].fd = connfd;
            break;
          }
        }
        if (i == kMaxReplicas) {
          // TODO(AD) Replica limit reached - log and refuse connections?
          // Do not add to active connections.
        }

        // TODO(AD) Must also poll for writable.
        client[i].events = POLLRDNORM;  // TODO also writable
        maxi = std::max(maxi, i);

        if (--nready <= 0) {
          continue;
        }
      }

      int sockfd;
      int n;
      for (i = 1; i <= maxi; ++i) {  // Check clients for data.
        if ((sockfd = client[i].fd) == -1) {
          continue;
        }

        if (client[i].revents & (POLLRDNORM | POLLERR)) {
          if ((n = read(sockfd, buf_.data(), kReadBufSize)) == -1) {
            if (errno == ECONNRESET) {
              close(sockfd);
              client[i].fd = -1;
            } else {
              // TODO(AD) Read error - Just log and close?
            }
          } else if (n == 0) {
            close(sockfd);
            client[i].fd = -1;
          } else {
            // TODO(AD) Handle incoming data.
          }

          if (--nready <= 0) {
            break;
          }
        }
      }
    }
  }

  void Listen() {
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(kListenPort);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if ((sock_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      throw LogException{"socket error"};
    }

    if (bind(sock_, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
      throw LogException{"bind error"};
    }

    if (listen(sock_, 10) == -1) {
      throw LogException{"listen error"};
    }
  }

  Log<S> log_;

  int sock_;

  std::vector<uint8_t> buf_;

  std::thread thread_;
  std::atomic_bool running_;
};

}  // namespace wombat::log
