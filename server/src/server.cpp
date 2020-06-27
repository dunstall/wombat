#include "server/server.h"

#include <netinet/in.h>
#include <netinet/ip.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdint>
#include <vector>

#include <glog/logging.h>
#include "log/logexception.h"
#include "record/producerecord.h"

namespace wombat::broker::server {

// TODO(AD) Duplication between Server and Leader - make both subclasses
Server::Server(uint16_t port, int max_clients)
    : port_{port}, fds_(max_clients), max_clients_{max_clients} {
  signal(SIGPIPE, SIG_IGN);
  LOG(INFO) << "starting server on port " << port;
  Listen();
}

std::vector<ProduceRecord> Server::Poll() {
  int ready = poll(fds_.data(), max_fd_index_ + 1, 0);
  if (ready == -1) {
    LOG(WARNING) << "leader poll error " << strerror(errno);
    return {};
  }

  if (PendingConnection()) {
    Accept();
    if (--ready <= 0) {
      return {};
    }
  }

  std::vector<ProduceRecord> recv{};
  for (int i = 1; i <= max_fd_index_; ++i) {
    if (PendingRead(i)) {
      std::vector<ProduceRecord> r = Read(i);
      recv.insert(recv.end(), r.begin(), r.end());
    }

    if (PendingWrite(i)) {
      // Write(i);
    }
  }

  return recv;
}

void Server::Listen() {
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
      connections_.emplace(connfd, Connection(connfd, cliaddr));
      return;
    }
  }

  LOG(WARNING) << "maximum clients reached (" << max_clients_ << ")";
  // Cannot accept so close immediately.
  close(connfd);
}

std::vector<ProduceRecord> Server::Read(int i) {
  int connfd = fds_[i].fd;
  Connection& conn = connections_.at(connfd);
  if (!conn.Read()) {
    connections_.erase(connfd);
    fds_[i].fd = -1;
  }
  return conn.Received();
}

bool Server::PendingConnection() const {
  return (fds_[0].revents & POLLRDNORM) != 0;
}

bool Server::PendingRead(int i) const {
  if (fds_[i].fd == -1) {
    return false;
  }
  return (fds_[i].revents & POLLRDNORM) != 0 || (fds_[i].revents & POLLERR) != 0;
}

bool Server::PendingWrite(int i) const {
  if (fds_[i].fd == -1) {
    return false;
  }
  return (fds_[i].revents & POLLWRNORM) != 0;
}

}  // namespace wombat::broker::server
