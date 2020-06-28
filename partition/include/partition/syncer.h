// Copyright 2020 Andrew Dunstall

#pragma once

namespace wombat::broker::partition {

template<class S>
class Syncer {
 public:
  virtual ~Syncer() {}

  virtual void Poll() = 0;
};

}  // namespace wombat::broker::partition
