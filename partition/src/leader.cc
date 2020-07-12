// Copyright 2020 Andrew Dunstall

#include "partition/leader.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>

#include "glog/logging.h"
#include "log/log.h"

namespace wombat::broker {

using namespace std::chrono_literals;  // NOLINT

Leader::Leader(uint32_t id,
               std::shared_ptr<Responder> responder,
               std::shared_ptr<log::Log> log)
    : Partition{id},
      produce_{id, log},
      consume_{id, log},
      stat_{id, log} {
  Start();
}

Leader::~Leader() {
  Stop();
}

void Leader::Poll() {
  while (running_) {
    const std::optional<Event> evt = events_.WaitForAndPop(50ms);
    if (evt) {
      Route(*evt);
    }
  }
}

void Leader::Route(const Event& evt) {
  // // TODO(AD) map request type -> Handler and add response to responder

  // switch (evt.message.type()) {
    // case frame::Type::kProduceRequest:
      // produce_.Handle(evt);
      // break;
    // case frame::Type::kConsumeRequest:
      // consume_.Handle(evt);
      // break;
    // case frame::Type::kStatRequest:
      // stat_.Handle(evt);
      // break;
    // default:
      // LOG(WARNING) << "leader received unrecognized message type "
          // << static_cast<int>(evt.message.type());
  // }
}

void Leader::Start() {
  running_ = true;
  thread_ = std::thread{&Leader::Poll, this};
}

void Leader::Stop() {
  running_ = false;
  if (thread_.joinable()) {
    thread_.join();
  }
}

}  // namespace wombat::broker
