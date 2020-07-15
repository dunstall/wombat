// Copyright 2020 Andrew Dunstall

#include "connection/connectionexception.h"

#include <cstring>
#include <exception>
#include <sstream>
#include <string>

namespace wombat::broker::connection {

ConnectionException::ConnectionException(const std::string& msg) : msg_{msg} {}

ConnectionException::ConnectionException(const std::string& msg, int err)
    : msg_{msg} {
  std::stringstream ss{};
  ss << msg;
  ss << ": " << std::strerror(err);
  ss << " (" << err << ")";
  msg_ = ss.str();
}

}  // namespace wombat::broker::connection
