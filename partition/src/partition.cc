// Copyright 2020 Andrew Dunstall

#include "partition/partition.h"

#include <cstdint>
#include <memory>
#include <thread>

#include "glog/logging.h"
#include "log/log.h"

namespace wombat::broker {

Partition::Partition(uint32_t id) : id_{id} {}

Partition::~Partition() {
  Stop();
}

void Partition::Handle(const Event& evt) {
  events_.Push(evt);
}

void Partition::Poll() {
  while (running_) {
    Process();
  }
}

void Partition::Start() {
  running_ = true;
  thread_ = std::thread{&Partition::Poll, this};
}

void Partition::Stop() {
  running_ = false;
  if (thread_.joinable()) {
    thread_.join();
  }
}

}  // namespace wombat::broker
