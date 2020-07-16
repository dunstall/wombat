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

	// Sets the value of the node at the given path. If the node does not exist
	// it will create it with the isEphemeral flag.
	Set(path string, data []byte, isEphemeral bool) error

	Delete(path string) error

	// Events returns a channel of updates. When a watched node changes true
	// is sent over the channel.
	//
	// Note must re-watch after every event.
	Events() <-chan bool

	Watch(path string) error

	Close()
}
