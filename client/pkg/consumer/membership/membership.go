package membership

import (
	"github.com/dunstall/wombatclient/pkg/consumer/registry"
)

type Membership struct{}

// This does not assign partitions, much call Rebalance() to get assignment.
func New(group string, id string, registry registry.Registry) (Membership, error) {
	_, err := newConsumerRegistry(group, id, registry)

	m := Membership{}
	return m, err
}

func (m *Membership) Assigned() []Chunk {
	return []Chunk{}
}

func (m *Membership) Rebalance() error {
	return nil
}

func (m *Membership) RequiresRebalance() <-chan bool {
	return make(chan bool)
}
