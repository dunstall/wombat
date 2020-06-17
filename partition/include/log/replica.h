#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#include <atomic>
#include <thread>

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
      : log_{log}, leader_{leader}
  {
    running_ = true;
    thread_ = std::thread{&Replica::Run, this};
  }

  ~Replica() {
    running_ = false;
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  Replica(const Replica&) = delete;
  Replica& operator=(const Replica&) = delete;

  Replica(Replica&&) = delete;
  Replica& operator=(Replica&&) = delete;

 private:
  void Run() {
    Connect();

    // TODO(AD) Create connection, send offset, listen for incoming
  }

  void Connect() {
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(leader_.port);
    if (inet_pton(AF_INET, leader_.ip.c_str(), &servaddr.sin_addr.s_addr) != 1) {
      throw LogException{"inet_pton error"};
    }

    if ((sock_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      throw LogException{"socket error"};
    }

    if (connect(sock_, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
      throw LogException{"failed to connect to server"};
    }

    // TODO 1 send offset
    // TODO 2 receive stream
    //
    // also handle reconnect
  }

  Log<S> log_;

  LeaderAddress leader_;

  int sock_;

  std::thread thread_;
  std::atomic_bool running_;
};

}  // namespace wombat::log
