// Copyright 2020 Andrew Dunstall

#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "server/event.h"

namespace wombat::broker::server {

class Responder {
 public:
  Responder();

  ~Responder();

  Responder(const Responder& conn) = delete;
  Responder& operator=(const Responder& conn) = delete;

  Responder(Responder&& conn) = delete;
  Responder& operator=(Responder&& conn) = delete;

  std::shared_ptr<EventQueue> events() const { return event_queue_; }

 private:
  void Poll();

  void Start();

  void Stop();

  std::shared_ptr<EventQueue> event_queue_;

  std::thread thread_;
  std::atomic_bool running_;
};

}  // namespace wombat::broker::server
