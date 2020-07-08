// Copyright 2020 Andrew Dunstall

#include "util/threadable.h"

#include <cstdint>
#include <memory>
#include <thread>

#include "util/pollable.h"

namespace wombat::broker::util {

Threadable::Threadable(std::shared_ptr<Pollable> poller)
    : poller_{std::move(poller)} {
  Start();
}

Threadable::~Threadable() {
  Stop();
}

void Threadable::Poll() {
  while (running_) {
    poller_->Poll();
  }
}

void Threadable::Start() {
  running_ = true;
  thread_ = std::thread{&Threadable::Poll, this};
}

void Threadable::Stop() {
  running_ = false;
  if (thread_.joinable()) {
    thread_.join();
  }
}

}  // namespace wombat::broker::util
