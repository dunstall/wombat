// Copyright 2020 Andrew Dunstall

#include "log/tempdir.h"

#include <iostream>
#include <filesystem>
#include <random>
#include <string>

namespace wombat::broker {

std::filesystem::path GeneratePath() {
  std::mt19937 rg{
    std::random_device {}()
  };
  std::uniform_int_distribution<std::string::size_type> d(
    0, CHARS.size() - 2
  );
  std::string dir;
  dir.reserve(PATH_LEN);
  for (size_t i = 0; i != PATH_LEN; ++i) {
    dir += CHARS[d(rg)];
  }
  return "/tmp/wombatlog" + dir;
}

TempDir::TempDir() : path_(GeneratePath()) {
  std::filesystem::create_directories(path_);
}

TempDir::~TempDir() {
  std::filesystem::remove_all(path_);
}

}  // namespace wombat::broker
