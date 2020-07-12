// Copyright 2020 Andrew Dunstall

#include "partition/replica.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

#include "glog/logging.h"
#include "log/log.h"

namespace wombat::broker {

using namespace std::chrono_literals;  // NOLINT

Replica::Replica(uint32_t id,
                 std::shared_ptr<Responder> responder,
                 std::shared_ptr<log::Log> log)
    : Partition{id},
      consume_{id, responder, log},
      stat_{id, responder, log} {
  Start();
}

Replica::~Replica() {
  Stop();
}

void Replica::Poll() {
  while (running_) {
    const std::optional<Event> evt = events_.WaitForAndPop(50ms);
    if (evt) {
      Route(*evt);
    }

    // TODO(AD) Poll leader
  }
}

// TODO(AD) Lots of duplication to leader - instead have a generic class
// with add handler method: AddHandler(handler, message type)
// Can move to Partition where only Poll is overridden
void Replica::Route(const Event& evt) {
  switch (evt.message.type()) {
    case frame::Type::kConsumeRequest:
      consume_.Handle(evt);
      break;
    case frame::Type::kStatRequest:
      stat_.Handle(evt);
      break;
    default:
      LOG(WARNING) << "replica received unrecognized message type "
          << static_cast<int>(evt.message.type());
  }
}

void Replica::Start() {
  running_ = true;
  thread_ = std::thread{&Replica::Poll, this};
}

void Replica::Stop() {
  running_ = false;
  if (thread_.joinable()) {
    thread_.join();
  }
}

}  // namespace wombat::broker
