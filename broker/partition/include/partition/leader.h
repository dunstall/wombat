// Copyright 2020 Andrew Dunstall

#pragma once

#include <cstdint>
#include <memory>

#include "log/log.h"
#include "partition/partition.h"
#include "server/responder.h"

namespace wombat::broker::partition {

class Leader : public Partition {
 public:
  Leader(uint32_t id, std::shared_ptr<server::Responder> responder,
         std::shared_ptr<log::Log> log);

  ~Leader() override;

  Leader(const Leader& conn) = delete;
  Leader& operator=(const Leader& conn) = delete;

  Leader(Leader&& conn) = delete;
  Leader& operator=(Leader&& conn) = delete;

 private:
  // TODO(AD) Use composition instead so Process can be unittested?
  // So pass Leader to partition
  void Process() override;
};

}  // namespace wombat::broker::partition
