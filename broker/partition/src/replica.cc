// Copyright 2020 Andrew Dunstall

#include "partition/replica.h"

#include <chrono>
#include <cstdint>
#include <memory>

#include "log/log.h"
#include "partition/consumehandler.h"
#include "partition/stathandler.h"
#include "server/responder.h"

namespace wombat::broker::partition {

using namespace std::chrono_literals;  // NOLINT

Replica::Replica(uint32_t id, std::shared_ptr<server::Responder> responder,
                 std::shared_ptr<log::Log> log)
    : Partition{id, responder} {
  router_.AddRoute(frame::Type::kConsumeRequest,
                   std::make_unique<ConsumeHandler>(id, log));
  router_.AddRoute(frame::Type::kStatRequest,
                   std::make_unique<StatHandler>(id, log));

  Start();
}

Replica::~Replica() { Stop(); }

void Replica::Process() {
  const std::optional<connection::Event> evt = events_.WaitForAndPop(50ms);
  if (evt) {
    router_.Route(*evt);
  }

  // TODO(AD) Poll leader
}

}  // namespace wombat::broker::partition
