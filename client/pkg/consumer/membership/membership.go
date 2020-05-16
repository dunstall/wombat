package membership

import (
	"path"
	"sort"
	"strconv"

	"github.com/dunstall/wombatclient/pkg/consumer/registry"
)

const (
	nPartitions = 15
)

type Membership struct {
	id       string
	group    string
	registry registry.Registry
	assigned []Chunk
}

// This does not assign partitions, much call Rebalance() to get assignment and
// start watching for changes.
func New(group string, id string, registry registry.Registry) (Membership, error) {
	_, err := newConsumerRegistry(group, id, registry)

	m := Membership{
		id,
		group,
		registry,
		[]Chunk{},
	}
	return m, err
}

func (m *Membership) Assigned() []Chunk {
	return m.assigned
}

func (m *Membership) AddTopic(topic string) error {
	p := path.Join("/", "partition", m.group, topic)
	return m.registry.Create(p, []byte{}, false)
}

// Rebalance the partitions among the consumer group. If cannot reach zookeeper
// an error is returned so should sleep and retry.
func (m *Membership) Rebalance() error {
	if err := m.clearAssigned(); err != nil {
		return err
	}

	consumers, err := m.consumers()
	if err != nil {
		return err
	}
	index := sort.SearchStrings(consumers, m.id)

	// TODO(AD) Hard code 15 partitions for now - this should be configurable
	// per topic (with global default)
	nAssigned := nPartitions / len(consumers)

	var from uint32 = uint32(nAssigned*index + 1)
	var to uint32 = uint32(nAssigned*(index+1) + 1)

	topics, err := m.topics()
	if err != nil {
		return err
	}
	for _, topic := range topics {
		for partition := from; partition != to; partition++ {
			if err := m.assign(Chunk{topic, partition}); err != nil {
				return err
			}
		}
	}

	return nil
}

func (m *Membership) RequiresRebalance() <-chan bool {
	return make(chan bool)
}

func (m *Membership) clearAssigned() error {
	for _, c := range m.assigned {
		p := path.Join("/", "partition", m.group, c.Topic, strconv.Itoa(int(c.Partition)))
		if err := m.registry.Delete(p); err != nil {
			return err
		}
	}
	m.assigned = []Chunk{}
	return nil
}

func (m *Membership) topics() ([]string, error) {
	topics, err := m.registry.GetRoot("/partition/" + m.group)
	if err != nil {
		return []string{}, err
	}
	sort.Strings(topics)
	return topics, nil
}

func (m *Membership) consumers() ([]string, error) {
	consumers, err := m.registry.GetRoot("/group/" + m.group)
	if err != nil {
		return []string{}, err
	}
	sort.Strings(consumers)
	return consumers, nil
}

func (m *Membership) assign(c Chunk) error {
	p := path.Join("/", "partition", m.group, c.Topic, strconv.Itoa(int(c.Partition)))
	if err := m.registry.CreateErrIfExist(p, []byte(m.id), true); err != nil {
		return err
	}
	m.assigned = append(m.assigned, c)
	return nil
}
