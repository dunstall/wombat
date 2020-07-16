// Copyright 2020 Andrew Dunstall

#pragma once

namespace wombat::broker::util {

class Pollable {
 public:
  virtual ~Pollable() {}

  virtual void Poll() = 0;
};

}  // namespace wombat::broker::util
