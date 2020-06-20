#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <vector>

#include <glog/logging.h>
#include "log/connection.h"
#include "log/log.h"
#include "log/logexception.h"

namespace wombat::log {

template<class S>
class Leader {
 public:
  Leader(std::shared_ptr<Log<S>> log, uint16_t port) : log_{log}, buf_(kReadBufSize) {
    LOG(INFO) << "starting leader on port " << port;
    Listen(port);
    InitPoll();
  }

  ~Leader() {
    Close();
  }

  Leader(const Leader&) = delete;
  Leader& operator=(const Leader&) = delete;

  Leader(Leader&&) = delete;
  Leader& operator=(Leader&&) = delete;

  void Poll() {
    int nready;
    if ((nready = poll(fds_, max_fd_index_ + 1, 0)) == -1) {
      throw LogException{"poll error", errno};
    }

    LOG(INFO) << nready;

    if (WaitingConnection()) {
      LOG(INFO) << "PENDING CONN";
      Accept();
      if (--nready <= 0) {
        return;
      }
    }

    for (int i = 1; i <= max_fd_index_; ++i) {  // Check clients for data.
      if (PendingRead(i)) {
        Read(i);
        if (--nready <= 0) {
          break;
        }
      }

      if (PendingWrite(i)) {
        uint32_t logsize = log_->size();
        uint32_t offset = connections_.at(fds_[i].fd).offset();

        LOG(INFO) << "PENDNIG WRITE " << logsize << " > " << offset;
        // TODO(AD) Must increment offset - test with unique data
        if (log_->size() > connections_.at(fds_[i].fd).offset()) {
          LOG(INFO) << "SENT " << log_->Send(connections_.at(fds_[i].fd).offset(), 100, fds_[i].fd);
        }
        // if (--nready <= 0) {
          // break;
        // }
      }
    }
  }

 private:
  static const int kListenBacklog = 10;
  static const int kMaxReplicas = 10;

  static const size_t kReadBufSize = 1000;

  void Listen(uint16_t port) {
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if ((listenfd_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      throw LogException{"socket error", errno};
    }

    if (bind(listenfd_, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
      throw LogException{"bind error", errno};
    }

    if (listen(listenfd_, kListenBacklog) == -1) {
      throw LogException{"listen error", errno};
    }
  }

  void Accept() {
    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);

    int connfd = accept(listenfd_, (struct sockaddr*) &cliaddr, &clilen);
    if (connfd == -1) {
      // TODO See accept(2) for errors to handle otherwise ignore
      // (make this a function and just skip)
    }

    LOG(INFO) << "replica connected";

    for (int i = 1; i != kMaxReplicas; ++i) {
      if (fds_[i].fd < 0) {
        fds_[i].fd = connfd;
        fds_[i].events = POLLRDNORM;
        max_fd_index_ = std::max(max_fd_index_, i);
        connections_.emplace(connfd, Connection(connfd, cliaddr));
        return;
      }
    }

    LOG(WARNING) << "maximum replicas reached";
    // Cannot accept so close immediately.
    close(connfd);
  }

  bool WaitingConnection() const {
    return (fds_[0].revents & POLLRDNORM) != 0;
  }

  bool PendingRead(int i) const {
    if (fds_[i].fd == -1) {
      return false;
    }
    return (fds_[i].revents & (POLLRDNORM | POLLERR)) != 0;
  }

  bool PendingWrite(int i) const {
    if (fds_[i].fd == -1) {
      return false;
    }
    return (fds_[i].revents & POLLWRNORM) != 0;
  }

  void Read(int i) {
    int sockfd = fds_[i].fd;
    if (!connections_.at(sockfd).Read()) {
      fds_[i].fd = -1;
      connections_.erase(sockfd);
      return;
    }

    // If established can listen for write.
    if (connections_.at(sockfd).state() == ConnectionState::kEstablished) {
      fds_[i].events = POLLRDNORM | POLLWRNORM;
    }
  }

  void InitPoll() {
    fds_[0].fd = listenfd_;
    fds_[0].events = POLLRDNORM;
    for (int i = 1; i < kMaxReplicas; ++i) {
      fds_[i].fd = -1;
    }
  }

  void Close() {
    close(listenfd_);
  }

  std::unordered_map<int, Connection> connections_;

  std::shared_ptr<Log<S>> log_;

  int listenfd_;

  std::vector<uint8_t> buf_;

  int max_fd_index_ = 0;

  struct pollfd fds_[kMaxReplicas];
};

}  // namespace wombat::log
