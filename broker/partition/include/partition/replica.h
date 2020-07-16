// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <memory>

#include "log/log.h"
#include "partition/partition.h"
#include "server/responder.h"

namespace wombat::broker::partition {

class Replica : public Partition {
 public:
  Replica(uint32_t id, std::shared_ptr<server::Responder> responder,
          std::shared_ptr<log::Log> log);

  ~Replica() override;

  Replica(const Replica& conn) = delete;
  Replica& operator=(const Replica& conn) = delete;

  Replica(Replica&& conn) = delete;
  Replica& operator=(Replica&& conn) = delete;

 private:
  void Process() override;
};

}  // namespace wombat::broker::partition
