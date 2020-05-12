package consumer

import (
	"fmt"

	"github.com/google/uuid"
	// TODO(AD) ZK should be hidden
	"github.com/samuel/go-zookeeper/zk"
)

type coordinator struct {
	sync *ZooKeeper
	// TODO must close and stop monitor thread
	updated chan bool
}

func newCoordinator(sync *ZooKeeper, group string) (coordinator, error) {
	coord := coordinator{
		sync,
		make(chan bool),
	}
	_, err := coord.register(group)
	if err != nil {
		return coord, err
	}

	if err = coord.rebalance(); err != nil {
		return coord, err
	}

	go coord.monitor(group)

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
	// TODO(AD) Either use Kafka range based - or look into something else like consistent hashing?
	// or just something simpler with zk - want to minimize rebalance

	// TODO(AD)
	fmt.Println("REBALANCE")
	return nil
}

func (coord *coordinator) updates() <-chan bool {
	return coord.updated
}

func (coord *coordinator) monitor(group string) {
	for {
		ch, err := coord.sync.WatchRegistry("/group/" + group)
		if err != nil {
			// TODO(AD)
		}

		<-ch
		coord.updated <- true
	}
}
