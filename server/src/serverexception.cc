// Copyright 2020 Andrew Dunstall

#include "server/serverexception.h"

/* #include <cstring> */
// #include <exception>
// #include <sstream>
// #include <string>

// #include "glog/logging.h"

namespace wombat::broker::server {

ServerException::ServerException(const std::string& msg) : msg_{msg} {
  // LOG(ERROR) << msg_;
}

ServerException::ServerException(const std::string& msg, int err) : msg_{msg} {
  /*   std::stringstream ss{}; */
  // ss << msg;
  // ss << ": " << std::strerror(err);
  // ss << " (" << err << ")";
  // msg_ = ss.str();
  /* LOG(ERROR) << msg_; */
}

}  // namespace wombat::broker::server
