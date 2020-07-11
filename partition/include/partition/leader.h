// Copyright 2020 Andrew Dunstall

#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "frame/record.h"
#include "frame/recordrequest.h"
#include "log/log.h"
#include "partition/consumehandler.h"
#include "partition/partition.h"
#include "partition/producehandler.h"
#include "partition/stathandler.h"

namespace wombat::broker {

class Leader : public Partition {
 public:
  Leader(uint32_t id,
         std::shared_ptr<Responder> responder,
         std::shared_ptr<log::Log> log);

  ~Leader() override;

  Leader(const Leader& conn) = delete;
  Leader& operator=(const Leader& conn) = delete;

  Leader(Leader&& conn) = delete;
  Leader& operator=(Leader&& conn) = delete;

 private:
  void Poll();

  void Route(const Event& evt);

  void Start();

  void Stop();

  ProduceHandler produce_;
  ConsumeHandler consume_;
  StatHandler stat_;

  std::thread thread_;
  std::atomic_bool running_;
};

}  // namespace wombat::broker
