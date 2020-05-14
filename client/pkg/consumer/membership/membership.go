package membership

import (
	"path"

	"github.com/dunstall/wombatclient/pkg/consumer/registry"
)

type Membership struct {
	group    string
	registry registry.Registry
}

// This does not assign partitions, much call Rebalance() to get assignment and
// start watching for changes.
func New(group string, id string, registry registry.Registry) (Membership, error) {
	_, err := newConsumerRegistry(group, id, registry)

	m := Membership{
		group,
		registry,
	}
	return m, err
}

func (m *Membership) Assigned() []Chunk {
	return []Chunk{}
}

func (m *Membership) AddTopic(topic string) error {
	p := path.Join("/", "partition", m.group, topic)
	return m.registry.Create(p, []byte{}, false)
}

func (m *Membership) Rebalance() error {
	return nil
}

func (m *Membership) RequiresRebalance() <-chan bool {
	return make(chan bool)
}
