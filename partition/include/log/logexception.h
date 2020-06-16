#pragma once

#include <exception>

namespace wombat::log {

class LogException : public std::exception {
 public:
  LogException(const std::string& msg) : msg_{msg} {}

  const char* what() const noexcept {
    return msg_.c_str();
  }

 private:
  std::string msg_;
};

}  // namespace wombat::log
