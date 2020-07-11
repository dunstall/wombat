// Copyright 2020 Andrew Dunstall

#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "event/event.h"
#include "log/log.h"
#include "partition/consumehandler.h"
#include "partition/partition.h"
#include "partition/stathandler.h"

namespace wombat::broker {

class Replica : public Partition {
 public:
  Replica(uint32_t id,
          std::shared_ptr<Responder> responder,
          std::shared_ptr<log::Log> log);

  ~Replica() override;

  Replica(const Replica& conn) = delete;
  Replica& operator=(const Replica& conn) = delete;

  Replica(Replica&& conn) = delete;
  Replica& operator=(Replica&& conn) = delete;

 private:
  void Poll();

  void Route(const Event& evt);

  void Start();

  void Stop();

  ConsumeHandler consume_;
  StatHandler stat_;

  std::thread thread_;
  std::atomic_bool running_;
};

}  // namespace wombat::broker
