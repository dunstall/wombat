// Copyright 2020 Andrew Dunstall

#include <memory>

#include "record/request.h"
#include "server/event.h"
#include "util/threadsafequeue.h"

namespace wombat::broker::server {

Event::Event(record::Request _request, std::shared_ptr<Connection> _connection)
    : request{_request}, connection{_connection} {
}

ResponseEvent::ResponseEvent(record::Response _response,
                             std::shared_ptr<Connection> _connection)
    : response{_response}, connection{_connection} {
}

}  // namespace wombat::broker::server
