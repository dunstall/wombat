package membership

import (
	"path"

	"github.com/dunstall/wombat/client/pkg/consumer/registry"
)

type consumerRegistry struct{}

func newConsumerRegistry(
	group string,
	id string,
	registry registry.Registry,
) (consumerRegistry, error) {
	p := path.Join("/", "group", group, id)
	err := registry.Create(p, []byte{}, true)
	return consumerRegistry{}, err
}
