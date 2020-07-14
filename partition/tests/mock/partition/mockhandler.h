// Copyright 2020 Andrew Dunstall

#pragma once

#include <optional>

#include "event/event.h"
#include "gmock/gmock.h"
#include "partition/handler.h"

namespace wombat::broker::partition {

class MockHandler : public Handler {
 public:
  MockHandler() : Handler(0) {}

  MOCK_METHOD(std::optional<Event>, Handle, (const Event& evt), (override));
};

}  // namespace wombat::broker::partition
