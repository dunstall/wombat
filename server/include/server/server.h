// Copyright 2020 Andrew Dunstall

#pragma once

#include <netinet/in.h>
#include <netinet/ip.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <unordered_map>
#include <vector>

#include "glog/logging.h"
#include "log/logexception.h"
#include "server/connection.h"
#include "util/threadsafequeue.h"

namespace wombat::broker::server {

// TODO(AD) use in Leader, ConsumeServer, ProduceServer (test well)
// write R to queue

template<class R>
class Server {
 public:
  explicit Server(uint16_t port, int max_clients = 1024)
      : port_{port},
        fds_(max_clients),
        max_clients_{max_clients},
        queue_{std::make_shared<util::ThreadSafeQueue<R>>()} {
    signal(SIGPIPE, SIG_IGN);
    LOG(INFO) << "starting server on port " << port;
    Listen();
  }

  ~Server() {
    Stop();
  }

  Server(const Server& conn) = delete;
  Server& operator=(const Server& conn) = delete;

  Server(Server&& conn) = default;
  Server& operator=(Server&& conn) = default;

  std::shared_ptr<util::ThreadSafeQueue<R>> queue() const { return queue_; }

  void Start() {
    running_ = true;
    thread_ = std::thread{&Server::Poll, this};
  }

  void Stop() {
    running_ = false;
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  void Poll() {
    while (running_) {
      // TODO(AD) if connections returned and writable - keep reference here so
      // can close and write etc.

      // TODO(AD) Try to block waiting rather than sleep
      // Wait for conn, readable, or connection with pending write (not socket
      // write)
      std::this_thread::sleep_for(std::chrono::milliseconds(50));

      int ready = poll(fds_.data(), max_fd_index_ + 1, 0);
      if (ready == -1) {
        LOG(WARNING) << "leader poll error " << strerror(errno);
        continue;
      }

      if (PendingConnection()) {
        Accept();
        if (--ready <= 0) {
          continue;
        }
      }

      for (int i = 1; i <= max_fd_index_; ++i) {
        if (PendingRead(i)) {
          Read(i);
        }

        if (PendingWrite(i)) {
          // Write(i);
        }
      }
    }
  }

 private:
  static const int kListenBacklog = 10;

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
    for (int i = 1; i != max_clients_ + 1; ++i) {
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

    for (int i = 1; i != max_clients_ + 1; ++i) {
      if (fds_[i].fd < 0) {
        fds_[i].fd = connfd;
        fds_[i].events = POLLRDNORM | POLLWRNORM | POLLERR;
        max_fd_index_ = std::max(max_fd_index_, i);
        connections_.emplace(connfd, Connection<R>(connfd, cliaddr));
        return;
      }
    }

    LOG(WARNING) << "maximum clients reached (" << max_clients_ << ")";
    // Cannot accept so close immediately.
    close(connfd);
  }

  void Read(int i) {
    int connfd = fds_[i].fd;
    Connection<R>& conn = connections_.at(connfd);
    if (!conn.Read()) {
      connections_.erase(connfd);
      fds_[i].fd = -1;
    }
    for (const R& r : conn.Received()) {
      queue_->Push(r);
    }
  }

  bool PendingConnection() const {
    return (fds_[0].revents & POLLRDNORM) != 0;
  }

  bool PendingRead(int i) const {
    if (fds_[i].fd == -1) {
      return false;
    }
    return (fds_[i].revents & POLLRDNORM) != 0
        || (fds_[i].revents & POLLERR) != 0;
  }

  bool PendingWrite(int i) const {
    if (fds_[i].fd == -1) {
      return false;
    }
    return (fds_[i].revents & POLLWRNORM) != 0;
  }

  uint16_t port_;

  int listenfd_;

  std::vector<struct pollfd> fds_;

  int max_fd_index_ = 0;

  std::unordered_map<int, Connection<R>> connections_;

  int max_clients_;

  std::thread thread_;
  std::atomic_bool running_;

  std::shared_ptr<util::ThreadSafeQueue<R>> queue_;
};

}  // namespace wombat::broker::server
