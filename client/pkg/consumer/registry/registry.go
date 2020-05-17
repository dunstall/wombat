package registry

import (
	"errors"
)

var (
	ErrNodeExists = errors.New("node exists")
	ErrNotFound   = errors.New("node not found")
)

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

	Close()
}
