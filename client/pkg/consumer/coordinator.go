package consumer

import (
	"fmt"

	"github.com/google/uuid"
	// TODO(AD) ZK should be hidden
	"github.com/samuel/go-zookeeper/zk"
)

type coordinator struct {
	sync *ZooKeeper
}

func newCoordinator(sync *ZooKeeper, group string) (coordinator, error) {
	coord := coordinator{sync}
	_, err := coord.register(group)
	if err != nil {
		return coord, err
	}

	if err = coord.rebalance(); err != nil {
		return coord, err
	}

	// TODO(AD) Start thread to watch /group/<group> and rebalance on update - the
	// actual rebalance must be in the main thread so just have channel indicating
	// rebalance needed.

	return coord, nil
}

func (coord *coordinator) register(group string) (string, error) {
	if err := coord.sync.AddRegistry("/group"); err != nil {
		return "", err
	}
	if err := coord.sync.AddRegistry("/group/" + group); err != nil {
		return "", err
	}

	id := uuid.New().String()
	if err := coord.sync.AddNode("/group/"+group+"/"+id, []byte{}, true); err != nil {
		// If this node exists try again with a new ID.
		if err == zk.ErrNodeExists {
			return coord.register(group)
		}
		return "", err
	}

	return id, nil
}

func (coord *coordinator) rebalance() error {
  // TODO(AD)
	fmt.Println("REBALANCE")
	return nil
}
