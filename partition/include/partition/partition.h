// Copyright 2020 Andrew Dunstall

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <thread>
#include <utility>

#include "glog/logging.h"
#include "log/log.h"
#include "partition/leader.h"
#include "partition/replica.h"
#include "partition/syncer.h"
#include "record/consumerecord.h"
#include "record/request.h"
#include "record/response.h"
#include "server/server.h"
#include "util/threadsafequeue.h"

namespace wombat::broker::partition {

// TODO(AD) A consumer is like a replica but pull rather than push (does replica
// need to be pull too, to avoid overloading as no backoff?)
template<class S>
class Partition {
 public:
  // TODO response queue
  Partition(std::shared_ptr<log::Log<S>> log,
            uint16_t port,
            std::unique_ptr<Syncer<S>> syncer)
      : log_{log}, syncer_{std::move(syncer)} {
    Start();
  }

  ~Partition() {
    Stop();
  }

  Partition(const Partition& conn) = delete;
  Partition& operator=(const Partition& conn) = delete;

  Partition(Partition&& conn) = default;
  Partition& operator=(Partition&& conn) = default;

  // TODO(AD) Have queue member for write only and read only - using same
  // structure (like Go channels)
  std::shared_ptr<server::EventQueue> queue() const { return queue_; }

 private:
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

  void Poll() {
    while (running_) {
      std::optional<server::Event> event;
      if ((event = queue_->TryPop()) && event) {
        HandleRequest(event->request, event->connection);
      }

      syncer_->Poll();
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }

  void HandleRequest(const record::Request& request,
                     std::shared_ptr<server::Connection> connection) {
    switch (request.type()) {
      case record::RequestType::kProduceRecord:
        Produce(request.payload());
        break;
      case record::RequestType::kConsumeRecord:
        std::optional<record::ConsumeRecord> record
            = record::ConsumeRecord::Decode(request.payload());
        if (!record) {
          // TODO(AD) Close connection as invalid request?
        }

        Produce(*record);
        break;
      default:
        LOG(WARNING) << "unknown request type";
        break;
    }
  }

  void Produce(const std::vector<uint8_t>& record) {
    // TODO(AD) Validate record (size) before appending
    log_->Append(record);
  }

  void Consume(const record::ConsumeRecord request) {
    std::optional<uint32_t> size = record::DecodeU32(
        log_->Lookup(request.offset(), 4)
    );
    // If the log is empty do nothing.
    if (!size) return;

    // Must succeed unless record is invalid.
    const std::vector<uint8_t> record = log_->Lookup(
        request.offset(), sizeof(uint32_t) + *size
    );

    record::Response response{record::ResponseType::kConsumeResponse, request};
    // TODO(AD) Push response
  }

  std::shared_ptr<log::Log<S>> log_;
  std::unique_ptr<Syncer<S>> syncer_;

  std::thread thread_;
  std::atomic_bool running_;

  std::shared_ptr<server::EventQueue> queue_;
};

}  // namespace wombat::broker::partition
