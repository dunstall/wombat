package consumer

import (
	"encoding/binary"
	"fmt"
	"time"

	"github.com/samuel/go-zookeeper/zk"
)

const (
	offsetRegistry = "/offsets"
)

type offsets struct {
	zkConn *zk.Conn
}

func newOffsets(servers []string, sessionTimeout time.Duration) (offsets, error) {
	zkConn, _, err := zk.Connect(servers, sessionTimeout)
	if err != nil {
		return offsets{}, fmt.Errorf("bad server config: %s", err)
	}

	o := offsets{zkConn}
	if err = o.addRegistry(); err != nil {
		return offsets{}, fmt.Errorf("failed to create registry: %s", err)
	}

	return o, nil
}

func (o *offsets) lookup(partition Partition) (uint64, error) {
	b, _, err := o.zkConn.Get(o.nodePath(partition))
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

	_, err := o.zkConn.Set(
		o.nodePath(partition), b, -1,
	)
	if err == zk.ErrNoNode {
		_, err = o.zkConn.Create(
			o.nodePath(partition), b, 0, zk.WorldACL(zk.PermRead|zk.PermWrite),
		)
	}
	return err
}

func (o *offsets) addRegistry() error {
	_, err := o.zkConn.Create(
		offsetRegistry,
		[]byte{},
		0,
		zk.WorldACL(zk.PermRead|zk.PermWrite|zk.PermCreate|zk.PermDelete),
	)
	// Ignore if the root has been created by another node.
	if err != nil && err != zk.ErrNodeExists {
		return err
	}
	return nil
}

func (o *offsets) nodePath(partition Partition) string {
	return offsetRegistry + "/" + partition.String()
}
