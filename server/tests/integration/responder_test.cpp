// Copyright 2020 Andrew Dunstall

#include <netinet/in.h>
#include <netinet/ip.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <cstdint>
#include <vector>

#include "gtest/gtest.h"
#include "server/connection.h"
#include "server/responder.h"

namespace wombat::broker::server {

class ResponderTest : public ::testing::Test {};

TEST_F(ResponderTest, TestSend) {
  // TODO(AD) Can use memfd_create as only fd needed in Connection
}

}  // namespace wombat::broker::server
