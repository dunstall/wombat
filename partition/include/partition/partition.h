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
#include "server/server.h"
#include "util/threadsafequeue.h"

namespace wombat::broker::partition {

// TODO(AD) Untested
template<class S>
class Partition {
 public:
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
        log_->Append(request.payload());
        break;
      case record::RequestType::kConsumeRecord:
        std::optional<record::ConsumeRecord> record
            = record::ConsumeRecord::Decode(request.payload());
        if (!record) {
          // TODO(AD) Close connection as invalid request?
        }

        std::optional<uint32_t> size = record::DecodeU32(
            log_.Lookup(record->offset(), 4)
        );
        if (!size) {
          // TODO(AD) Assume log empty
        }

        // TODO(AD) Handle keep sending until all sent. Also queue data sent
        // to client to avoid interleaving.
        log_.Send(
            record->offset(), sizeof(uint32_t) + *size, connection->connfd()
        );
        break;
      default:
        LOG(WARNING) << "unknown request type";
        break;
    }
  }

  std::shared_ptr<log::Log<S>> log_;
  std::unique_ptr<Syncer<S>> syncer_;

  std::thread thread_;
  std::atomic_bool running_;

  std::shared_ptr<server::EventQueue> queue_;
};

}  // namespace wombat::broker::partition
