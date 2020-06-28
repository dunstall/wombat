// Copyright 2020 Andrew Dunstall

#pragma once

#include <filesystem>

namespace wombat::broker {

class Broker {
 public:
  explicit Broker(const std::filesystem::path& config_path);
};

}  // namespace wombat::broker
