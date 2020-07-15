// Copyright 2020 Andrew Dunstall

#pragma once

#include <memory>
#include <unordered_map>

#include "connection/event.h"
#include "server/responder.h"
#include "frame/messageheader.h"
#include "partition/handler.h"

namespace wombat::broker::partition {

class Router {
 public:
  explicit Router(std::shared_ptr<server::Responder> responder);

  Router(const Router& conn) = delete;
  Router& operator=(const Router& conn) = delete;

  Router(Router&& conn) = default;
  Router& operator=(Router&& conn) = default;

  void Route(const connection::Event& evt) const;

  void AddRoute(frame::Type type, std::unique_ptr<Handler> handler);

 private:
  std::shared_ptr<server::Responder> responder_;

  std::unordered_map<frame::Type, std::unique_ptr<Handler>> handlers_;
};

}  // namespace wombat::broker::partition
