// Copyright 2020 Andrew Dunstall

#pragma once

// #include <cstring>
// #include <exception>
// #include <sstream>
#include <string>

namespace wombat::broker::server {

class ServerException : public std::exception {
 public:
  explicit ServerException(const std::string& msg);

  ServerException(const std::string& msg, int err);

  const char* what() const noexcept { return msg_.c_str(); }

 private:
  std::string msg_;
};

}  // namespace wombat::broker::server