// Copyright 2020 Andrew Dunstall

#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "util/pollable.h"

namespace wombat::broker::util {

class Threadable {
 public:
  explicit Threadable(std::unique_ptr<Pollable> poller);

  ~Threadable();

  Threadable(const Threadable& conn) = delete;
  Threadable& operator=(const Threadable& conn) = delete;

  Threadable(Threadable&& conn) = delete;
  Threadable& operator=(Threadable&& conn) = delete;

 private:
  void Poll();

  void Start();

  void Stop();

  std::thread thread_;
  std::atomic_bool running_;

  std::unique_ptr<Pollable> poller_;
};

}  // namespace wombat::broker::util
