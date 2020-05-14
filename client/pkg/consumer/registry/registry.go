package registry

import (
  "errors"
)

var (
	ErrNodeExists = errors.New("node exists")
)

type Event struct{}

type Registry interface {
  // Create creates a new node at the given path. This creates the root if it
  // does not exist.
	Create(path string, data []byte, isEphemeral bool) error

	Get(path string) ([]byte, error)

	Set(path string, data []byte) error

	Delete(path string) error

  // Watch returns a channel of notification about updates to the nodes in the
  // given root. If any nodes change true is sent over the channel.
	Watch(root string) (<-chan bool, error)

  // ConnEvents returns a channel of events about the connection.
  // TODO(AD) keep this internal? just return error if not connected then app just
  // keep retrying
	ConnEvents() <-chan Event

	Close()
}
