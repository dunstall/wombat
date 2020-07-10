// Copyright 2020 Andrew Dunstall

#include "event/event.h"
#include "event/responder.h"

namespace wombat::broker {

void Responder::Respond(const Event& evt) {
  // TODO(AD) This must push to a queue that is handled in a background thread
  // that keeps attempting to write until fully sent and avoid MULTIPLE
  // THREADS writing to same connection at once.
  // For now just write.
  evt.connection->Send(evt.message.Encode());
}

}  // namespace wombat::broker
