// Copyright 2020 Andrew Dunstall

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>

#include "log/log.h"
#include "partition/leader.h"
#include "record/consumerecord.h"
#include "record/producerecord.h"
#include "util/threadsafequeue.h"

namespace wombat::broker::partition {

enum class PartitionType {
  kLeader,
  kReplica
};

template<class S>
class PartitionLeader {
 public:
  PartitionLeader(std::shared_ptr<log::Log<S>> log, uint16_t port)
      : log_{log}, leader_{log, port} {}

  ~PartitionLeader() {
    Stop();
  }

  PartitionLeader(const PartitionLeader& conn) = delete;
  PartitionLeader& operator=(const PartitionLeader& conn) = delete;

  PartitionLeader(PartitionLeader&& conn) = default;
  PartitionLeader& operator=(PartitionLeader&& conn) = default;

  // TODO(AD) Have queue member for write only and read only - using same
  // structure (like Go channels)
  std::shared_ptr<util::ThreadSafeQueue<record::ProduceRecord>> queue() const {
    return queue_;
  }

  void Start() {
    running_ = true;
    thread_ = std::thread{&PartitionLeader::Poll, this};
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
      std::optional<record::ProduceRecord> record;
      while ((record = queue_->TryPop()) && record) {
        log_->Append((*record).Encode());
      }

      leader_.Poll();
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }

  std::shared_ptr<log::Log<S>> log_;
  Leader<S> leader_;

  std::thread thread_;
  std::atomic_bool running_;

  std::shared_ptr<util::ThreadSafeQueue<record::ProduceRecord>> queue_;
};

}  // namespace wombat::broker::partition
