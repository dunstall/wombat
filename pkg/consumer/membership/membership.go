package membership

import (
	"encoding/binary"
	"path"
	"sort"
	"strconv"

	"github.com/dunstall/wombatclient/pkg/consumer/registry"
	"github.com/golang/glog"
	"github.com/samuel/go-zookeeper/zk"
)

// TODO(AD) logging

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
	if err := m.registry.CreateRoot(p); err != nil && err != zk.ErrNodeExists {
		return err
	}
	return nil
}

// Rebalance the partitions among the consumer group. If cannot reach zookeeper
// an error is returned so should sleep and retry.
// TODO(AD) Caller must watch the registry and call on update
func (m *Membership) Rebalance() error {
	glog.Infof("rebalance")

	if err := m.clearAssigned(); err != nil {
		return err
	}

	topics, err := m.topics()
	if err != nil {
		return err
	}

	consumers, err := m.consumers()
	if err != nil {
		return err
	}

	for _, topic := range topics {
		if err := m.assignPartitions(topic, consumers); err != nil {
			return err
		}
	}

	return nil
}

func (m *Membership) GetOffset(c Chunk) (uint64, error) {
	p := path.Join("/", "offset", m.group, c.Topic, strconv.Itoa(int(c.Partition)))
	b, err := m.registry.Get(p)
	if err == zk.ErrNoNode {
		glog.Infof("offset %s not found - default to 0", p)
		return 0, nil
	}
	if err != nil {
		return 0, err
	}
	return binary.BigEndian.Uint64(b[:8]), nil
}

func (m *Membership) CommitOffset(offset uint64, c Chunk) error {
	p := path.Join("/", "offset", m.group, c.Topic, strconv.Itoa(int(c.Partition)))
	b := make([]byte, 8)
	binary.BigEndian.PutUint64(b, offset)
	return m.registry.Set(p, b, false)
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
	return consumers, nil
}

func (m *Membership) assignPartitions(topic string, consumers []string) error {
	partitions := partitionRange(m.id, consumers, nPartitions)
	for _, partition := range partitions {
		if err := m.assignPartition(Chunk{topic, partition}); err != nil {
			return err
		}
	}
	return nil
}

func (m *Membership) assignPartition(c Chunk) error {
	p := path.Join("/", "partition", m.group, c.Topic, strconv.Itoa(int(c.Partition)))
	if err := m.registry.Create(p, []byte(m.id), true); err != nil {
		return err
	}
	m.assigned = append(m.assigned, c)
	return nil
}

func partitionRange(id string, consumers []string, nPartitions int) []uint32 {
	sort.Strings(consumers)
	index := sort.SearchStrings(consumers, id)
	partitions := []uint32{}
	// TODO(AD) Hard code 15 partitions for now - this should be configurable
	// per topic (with global default)
	for p := index + 1; p <= nPartitions; p += len(consumers) {
		partitions = append(partitions, uint32(p))
	}
	return partitions
}
