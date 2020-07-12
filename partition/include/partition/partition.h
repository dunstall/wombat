// Copyright 2020 Andrew Dunstall

#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>

#include "event/event.h"
#include "partition/router.h"

namespace wombat::broker {

class Partition {
 public:
  explicit Partition(uint32_t id);

  virtual ~Partition();

  Partition(const Partition& conn) = delete;
  Partition& operator=(const Partition& conn) = delete;

  Partition(Partition&& conn) = delete;
  Partition& operator=(Partition&& conn) = delete;

  uint32_t id() const { return id_; }

  virtual void Handle(const Event& evt);

 protected:
  void Poll();

  virtual void Process() = 0;

  void Start();

  void Stop();

  EventQueue events_;

  uint32_t id_;

  std::thread thread_;
  std::atomic_bool running_;

  Router router_;
};

}  // namespace wombat::broker
