// Copyright 2020 Andrew Dunstall

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <utility>

#include "log/log.h"
#include "partition/leader.h"
#include "partition/replica.h"
#include "partition/syncer.h"
#include "record/request.h"
#include "util/threadsafequeue.h"

namespace wombat::broker::partition {

// TODO(AD) Untested
template<class S>
class Partition {
 public:
  Partition(std::shared_ptr<log::Log<S>> log,
            uint16_t port,
            std::unique_ptr<Syncer<S>> syncer)
      : log_{log}, syncer_{std::move(syncer)} {}

  ~Partition() {
    Stop();
  }

  Partition(const Partition& conn) = delete;
  Partition& operator=(const Partition& conn) = delete;

  Partition(Partition&& conn) = default;
  Partition& operator=(Partition&& conn) = default;

  // TODO(AD) Have queue member for write only and read only - using same
  // structure (like Go channels)
  std::shared_ptr<util::ThreadSafeQueue<record::Request>> queue() const {
    return queue_;
  }

  void Start() {
    running_ = true;
    thread_ = std::thread{&Partition::Poll, this};
  }

  void Stop() {
    running_ = false;
    if (thread_.joinable()) {
      thread_.join();
    }
  }

 private:
  void Poll() {
    while (running_) {
      std::optional<record::Request> request;
      while ((request = queue_->TryPop()) && request) {
        // TODO(AD) Handle - check type and decode
        // log_->Append((*record).Encode());
      }

      syncer_->Poll();
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }

  std::shared_ptr<log::Log<S>> log_;
  // TODO(AD) both leader and replica - make siblings? Syncer?
  std::unique_ptr<Syncer<S>> syncer_;

  std::thread thread_;
  std::atomic_bool running_;

  std::shared_ptr<util::ThreadSafeQueue<record::Request>> queue_;
};

}  // namespace wombat::broker::partition
