// Copyright 2020 Andrew Dunstall

#include "event/connection.h"

namespace wombat::broker {

Connection::Connection(int connfd, const struct sockaddr_in& addr)
    : connfd_(connfd) {}

Connection::~Connection() {}

}  // namespace wombat::broker
