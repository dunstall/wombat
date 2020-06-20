#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdint>
#include <memory>

#include <glog/logging.h>
#include "log/log.h"
#include "log/logexception.h"
#include "partition/connection.h"

namespace wombat::log {

template<class S>
class Leader {
 public:
  Leader(std::shared_ptr<Log<S>> log, uint16_t port) : port_{port} {
    LOG(INFO) << "starting leader on port " << port;
    Listen();
  }

  void Poll() {

    int ready;
    ready = poll(fds_, max_fd_index_ + 1, 0);
    if (ready == -1) {
      LOG(WARNING) << "leader poll error " << strerror(errno);
      return;
    }

    if (PendingConnection()) {
      Accept();
      if (--ready <= 0) {
        return;
      }
    }
  }

 private:
  static const int kListenBacklog = 10;
  static const int kMaxReplicas = 10;

  void Listen() {
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if ((listenfd_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      LOG(ERROR) << "failed to create socket " << strerror(errno);
      throw LogException{"socket error", errno};
    }

    if (bind(listenfd_, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
      LOG(ERROR) << "failed to bind socket " << strerror(errno);
      throw LogException{"bind error", errno};
    }

    if (listen(listenfd_, kListenBacklog) == -1) {
      LOG(ERROR) << "failed to listen socket " << strerror(errno);
      throw LogException{"listen error", errno};
    }

    fds_[0].fd = listenfd_;
    fds_[0].events = POLLRDNORM;
    for (int i = 1; i != kMaxReplicas + 1; ++i) {
      fds_[i].fd = -1;
    }
  }

  void Accept() {
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    int connfd = accept(listenfd_, (struct sockaddr*) &cliaddr, &clilen);
    if (connfd == -1) {
      LOG(WARNING) << "failed to accept connection " << strerror(errno);
      return;
    }

    for (int i = 1; i != kMaxReplicas + 1; ++i) {
      if (fds_[i].fd < 0) {
        fds_[i].fd = connfd;
        fds_[i].events = POLLRDNORM;
        max_fd_index_ = std::max(max_fd_index_, i);
        // connections_.emplace(connfd, Connection(connfd, cliaddr)); TODO
        Connection(connfd, cliaddr);
        return;
      }
    }

    LOG(WARNING) << "maximum replicas reached (" << kMaxReplicas << ")";
    // Cannot accept so close immediately.
    close(connfd);
  }

  bool PendingConnection() const {
    return (fds_[0].revents & POLLRDNORM) != 0;
  }

  uint16_t port_;

  int listenfd_;

  struct pollfd fds_[kMaxReplicas + 1];

  int max_fd_index_ = 0;
};

}  // namespace wombat::log
