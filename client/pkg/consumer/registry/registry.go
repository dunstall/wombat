package registry

import (
	"errors"
)

const (
	// JoinedSession indicates the node connected and joined a session.
	JoinedSession EventType = 1
	// ConnectionLost indicates the nodes connection was dropped.
	ConnectionLost EventType = 2
	// SessionExpired indicates the config service expired the nodes current
	// session while it was disconnected.
	SessionExpired EventType = 3
)

var (
	ErrNodeExists = errors.New("node exists")
	ErrNotFound   = errors.New("node not found")
)

type EventType int32

// Event represents a notification from the configuration service.
type Event struct {
	Type EventType
	Path string
}

type Registry interface {
	// Create creates a new node at the given path. This creates the root if it
	// does not exist.
	Create(path string, data []byte, isEphemeral bool) error

  CreateRoot(p string) error

	Get(path string) ([]byte, error)

	GetRoot(path string) ([]string, error)

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
