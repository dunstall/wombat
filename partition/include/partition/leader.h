// Copyright 2020 Andrew Dunstall

#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <unordered_map>

#include "glog/logging.h"
#include "log/log.h"
#include "log/logexception.h"
#include "partition/connection.h"
#include "partition/syncer.h"

namespace wombat::broker::partition {

// TODO(AD) Use generic server. Listen for offset request to get a connection
// object to write to.
template<class S>
class Leader : public Syncer<S> {
 public:
  Leader(std::shared_ptr<log::Log<S>> log, uint16_t port)
      : port_{port}, log_{log}, connections_{} {
    signal(SIGPIPE, SIG_IGN);
    LOG(INFO) << "starting leader on port " << port;
    Listen();
  }

  void Poll() override {
    int ready = poll(fds_, max_fd_index_ + 1, 0);
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

    for (int i = 1; i <= max_fd_index_; ++i) {
      if (PendingRead(i)) {
        Read(i);
      }

      if (PendingWrite(i)) {
        Write(i);
      }
    }
  }

 private:
  static const int kListenBacklog = 10;
  static const int kMaxReplicas = 10;
  static const int kMaxSend = 1024;

  void Listen() {
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if ((listenfd_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      LOG(ERROR) << "failed to create socket " << strerror(errno);
      throw log::LogException{"socket error", errno};
    }

    if (bind(listenfd_, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
      LOG(ERROR) << "failed to bind socket " << strerror(errno);
      throw log::LogException{"bind error", errno};
    }

    if (listen(listenfd_, kListenBacklog) == -1) {
      LOG(ERROR) << "failed to listen socket " << strerror(errno);
      throw log::LogException{"listen error", errno};
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
        fds_[i].events = POLLRDNORM | POLLWRNORM | POLLERR;
        max_fd_index_ = std::max(max_fd_index_, i);
        connections_.emplace(connfd, Connection(connfd, cliaddr));
        return;
      }
    }

    LOG(WARNING) << "maximum replicas reached (" << kMaxReplicas << ")";
    // Cannot accept so close immediately.
    close(connfd);
  }

  void Read(int i) {
    int connfd = fds_[i].fd;
    Connection& conn = connections_.at(connfd);
    if (!conn.Read()) {
      connections_.erase(connfd);
      fds_[i].fd = -1;
    }
  }

  void Write(int i) {
    int connfd = fds_[i].fd;
    Connection& conn = connections_.at(connfd);

    if (conn.offset() >= log_->size()) {
      return;
    }

    // TODO(AD) Need to worry about race (append and send at same time)?
    uint32_t written = log_->Send(conn.offset(), kMaxSend, connfd);
    conn.set_offset(conn.offset() + written);

    LOG(INFO) << "written " << written << " bytes to " << conn.address();
  }

  bool PendingConnection() const {
    return (fds_[0].revents & POLLRDNORM) != 0;
  }

  bool PendingRead(int i) const {
    if (fds_[i].fd == -1) {
      return false;
    }

    return (fds_[i].revents & POLLRDNORM) != 0 ||
        (fds_[i].revents & POLLERR) != 0;
  }

  bool PendingWrite(int i) const {
    if (fds_[i].fd == -1) {
      return false;
    }

    int connfd = fds_[i].fd;
    // Only write to the replica once its offset has been received.
    if (connections_.at(connfd).state() != ConnectionState::kEstablished) {
      return false;
    }

    return (fds_[i].revents & POLLWRNORM) != 0;
  }

  uint16_t port_;

  int listenfd_;

  struct pollfd fds_[kMaxReplicas + 1];

  int max_fd_index_ = 0;

  std::shared_ptr<log::Log<S>> log_;

  std::unordered_map<int, Connection> connections_;
};

}  // namespace wombat::broker::partition
