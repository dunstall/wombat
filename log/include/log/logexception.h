#pragma once

#include <cstring>
#include <exception>
#include <sstream>
#include <string>

#include <glog/logging.h>

namespace wombat::broker {

class LogException : public std::exception {
 public:
  LogException(const std::string& msg) : msg_{msg} {
    LOG(ERROR) << msg_;
  }

  LogException(const std::string& msg, int err) : msg_{msg} {
    std::stringstream ss{};
    ss << msg;
    ss << ": " << std::strerror(err);
    ss << " (" << err << ")";
    msg_ = ss.str();
    LOG(ERROR) << msg_;
  }

  const char* what() const noexcept {
    return msg_.c_str();
  }

 private:
  std::string msg_;
};

}  // namespace wombat::broker
