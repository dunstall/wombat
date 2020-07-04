// Copyright 2020 Andrew Dunstall

#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "partition/partition.h"
#include "partition/syncer.h"
#include "server/server.h"

namespace wombat::broker::partition {

template<class S>
class Listener {
 public:
  Listener(Partition<S> partition,
           std::unique_ptr<Syncer<S>> syncer,
           std::shared_ptr<server::ResponseEventQueue> responses)
      : partition_{partition},
        syncer_{std::move(syncer)},
        responses_{responses} {
    Start();
  }

  ~Listener() {
    Stop();
  }

  Listener(const Listener& conn) = delete;
  Listener& operator=(const Listener& conn) = delete;

  Listener(Listener&& conn) = delete;
  Listener& operator=(Listener&& conn) = delete;

  std::shared_ptr<server::EventQueue> queue() const { return queue_; }

 private:
  void Start() {
    running_ = true;
    thread_ = std::thread{&Listener::Poll, this};
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
      while ((event = queue_->TryPop()) && event) {
        const auto resp = partition_.Handle(event->request);
        if (resp) {
          responses_->Push(server::ResponseEvent{*resp, event->connection});
        }
      }

      syncer_->Poll();
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
  }

  Partition<S> partition_;

  std::unique_ptr<Syncer<S>> syncer_;

  std::thread thread_;
  std::atomic_bool running_;

  std::shared_ptr<server::EventQueue> queue_;

  std::shared_ptr<server::ResponseEventQueue> responses_;
};

}  // namespace wombat::broker::partition
