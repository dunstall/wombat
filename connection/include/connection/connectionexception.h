// Copyright 2020 Andrew Dunstall

#pragma once

#include <exception>
#include <string>

namespace wombat::broker::connection {

class ConnectionException : public std::exception {
 public:
  explicit ConnectionException(const std::string& msg) : msg_{msg} {};

  const char* what() const noexcept { return msg_.c_str(); }

 private:
  std::string msg_;
};

}  // namespace wombat::broker::connection
