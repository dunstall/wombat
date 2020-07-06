// Copyright 2020 Andrew Dunstall

#include "server/responder.h"

#include <atomic>
#include <thread>

#include "glog/logging.h"

namespace wombat::broker::server {

// TODO(AD) In utils create generic threaded object with start, stop and
// virtual poll

Responder::Responder() : event_queue_{std::make_shared<EventQueue>()} {
  Start();
}

Responder::~Responder() {
  Stop();
}

void Responder::Start() {
  running_ = true;
  thread_ = std::thread{&Responder::Poll, this};
}

void Responder::Stop() {
  running_ = false;
  if (thread_.joinable()) {
    thread_.join();
  }
}

void Responder::Poll() {
  while (running_) {
    const std::optional<Event> evt = event_queue_->TryPop();
    if (evt) {
      evt->connection->Send(evt->message.Encode());
    }

    // TODO(AD) Handle partial writes - keep set of pending connections

    std::this_thread::sleep_for(std::chrono::milliseconds(50));  // TODO(AD)
  }
}

}  // namespace wombat::broker::server
