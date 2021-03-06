// Copyright 2020 Andrew Dunstall

#include "partition/handler.h"

#include <cstdint>

namespace wombat::broker::partition {

Handler::Handler(uint32_t id) : id_{id} {}

}  // namespace wombat::broker::partition
