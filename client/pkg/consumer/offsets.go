package consumer

import (
	"encoding/binary"
	"fmt"

	// TODO(AD) ZK should be hidden
	"github.com/samuel/go-zookeeper/zk"
)

const (
	offsetRegistry = "/offset"
)

type offsets struct {
	sync  *ZooKeeper
	group string
}

func newOffsets(sync *ZooKeeper, group string) (offsets, error) {
	o := offsets{sync, group}
	if err := o.addRegistry(); err != nil {
		return offsets{}, fmt.Errorf("failed to create registry: %s", err)
	}
	if err := o.addGroup(); err != nil {
		return offsets{}, fmt.Errorf("failed to create registry: %s", err)
	}

	return o, nil
}

func (o *offsets) lookup(partition Partition) (uint64, error) {
	b, err := o.sync.GetNode(o.nodePath(partition))
	if err == zk.ErrNoNode {
		return 0, nil
	}
	if err != nil {
		return 0, err
	}

	return binary.BigEndian.Uint64(b[:8]), nil
}

func (o *offsets) commit(partition Partition, offset uint64) error {
	b := make([]byte, 8)
	binary.BigEndian.PutUint64(b, offset)
	return o.sync.SetNode(o.nodePath(partition), b, false)
}

func (o *offsets) addGroup() error {
	return o.sync.AddRegistry(offsetRegistry + "/" + o.group)
}

func (o *offsets) addRegistry() error {
	return o.sync.AddRegistry(offsetRegistry)
}

func (o *offsets) nodePath(partition Partition) string {
	return offsetRegistry + "/" + o.group + "/" + partition.String()
}
