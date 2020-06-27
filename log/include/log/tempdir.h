// Copyright 2020 Andrew Dunstall

#pragma once

#include <iostream>
#include <filesystem>
#include <string>

namespace wombat::broker::log {

const size_t PATH_LEN = 6;
const std::string CHARS = "0123456789abcdefghijklmnopqrstuvwxyz";  // NOLINT

std::filesystem::path GeneratePath();

class TempDir {
 public:
  TempDir();

  ~TempDir();

  std::filesystem::path path() const { return path_; }

 private:
  std::filesystem::path path_;
};

}  // namespace wombat::broker::log
