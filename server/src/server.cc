// Copyright 2020 Andrew Dunstall

#include "server/server.h"

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
#include <vector>

#include "glog/logging.h"
#include "server/serverexception.h"
#include "server/tcpconnection.h"

namespace wombat::broker::server {

Server::Server(uint16_t port, int max_clients)
    : port_{port},
      fds_(max_clients),
      max_clients_{max_clients},
      event_queue_{std::make_shared<EventQueue>()} {
  signal(SIGPIPE, SIG_IGN);
  LOG(INFO) << "server listening on port " << port;
  Listen();
}

void Server::Poll() {
  int ready = poll(fds_.data(), max_fd_index_ + 1, kPollTimeoutMS);
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
  }
}

void Server::Listen() {
  struct sockaddr_in servaddr;
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port_);
  servaddr.sin_addr.s_addr = INADDR_ANY;

  if ((listenfd_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    LOG(ERROR) << "failed to create socket " << strerror(errno);
    throw ServerException{"socket error", errno};
  }

  if (bind(listenfd_, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
    LOG(ERROR) << "failed to bind socket " << strerror(errno);
    throw ServerException{"bind error", errno};
  }

  if (listen(listenfd_, kListenBacklog) == -1) {
    LOG(ERROR) << "failed to listen socket " << strerror(errno);
    throw ServerException{"listen error", errno};
  }

  fds_[0].fd = listenfd_;
  fds_[0].events = POLLRDNORM;
  for (int i = 1; i != max_clients_ + 1; ++i) {
    fds_[i].fd = -1;
  }
}

void Server::Accept() {
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
      connections_.emplace(
          connfd, std::make_shared<TcpConnection>(connfd, cliaddr)
      );
      return;
    }
  }

  LOG(WARNING) << "maximum clients reached (" << max_clients_ << ")";
  // Cannot accept so close immediately.
  close(connfd);
}

void Server::Read(int i) {
  int connfd = fds_[i].fd;
  std::shared_ptr<Connection> conn = connections_.at(connfd);
  try {
    std::optional<frame::Message> request = conn->Receive();
    if (request) {
      Event e{*request, conn};
      event_queue_->Push(e);
    }
  } catch (const ConnectionException& e) {
    connections_.erase(connfd);
    fds_[i].fd = -1;
  }
}

bool Server::PendingConnection() const {
  return (fds_[0].revents & POLLRDNORM) != 0;
}

bool Server::PendingRead(int i) const {
  if (fds_[i].fd == -1) {
    return false;
  }
  return (fds_[i].revents & POLLRDNORM) != 0
      || (fds_[i].revents & POLLERR) != 0;
}

bool Server::PendingWrite(int i) const {
  if (fds_[i].fd == -1) {
    return false;
  }
  return (fds_[i].revents & POLLWRNORM) != 0;
}

}  // namespace wombat::broker::server
