package consumer

import (
	"fmt"
	"sort"
	"strconv"

	"github.com/google/uuid"
	// TODO(AD) ZK should be hidden
	"github.com/samuel/go-zookeeper/zk"
)

type coordinator struct {
	sync *ZooKeeper
	// TODO must close and stop monitor thread
	updated  chan bool
	id       string
	assigned map[string][]string
}

func newCoordinator(sync *ZooKeeper, group string) (coordinator, error) {
	coord := coordinator{
		sync:     sync,
		updated:  make(chan bool),
		assigned: make(map[string][]string),
	}
	_, err := coord.register(group)
	if err != nil {
		return coord, err
	}

	if err = coord.rebalance(group); err != nil {
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
	coord.id = id
	if err := coord.sync.AddNode("/group/"+group+"/"+id, []byte{}, true); err != nil {
		// If this node exists try again with a new ID.
		if err == zk.ErrNodeExists {
			return coord.register(group)
		}
		return "", err
	}

	return id, nil
}

func (coord *coordinator) addTopic(group string, topic string) error {

	if err := coord.sync.AddRegistry("/partition"); err != nil {
		return err
	}
	if err := coord.sync.AddRegistry("/partition/" + group); err != nil {
		return err
	}
	if err := coord.sync.AddRegistry("/partition/" + group + "/" + topic); err != nil {
		return err
	}

	return nil
}

func (coord *coordinator) rebalance(group string) error {

	// /partitions/[group]/[topic]/[partition] -> [consumer UUID]

	// TODO remove any owned partitions
	// for partition in assigned
	// add ephemeral partition -> UUID

	for topic, partitions := range coord.assigned {
		for _, partition := range partitions {
			if err := coord.sync.DeleteNode("/partition/" + group + "/" + topic + "/" + partition); err != nil {
				// TODO
			}
		}
		// TODO remove
	}

	topics, err := coord.sync.GetRegistry("/partition/" + group)
	if err != nil {
		return err
	}

	consumers, err := coord.sync.GetRegistry("/group/" + group)
	if err != nil {
		return err
	}
	// fmt.Println(consumers)

	partitions := []string{
		"1", "2", "3", "4", "5",
		"6", "7", "8", "9", "10",
		"11", "12", "13", "14", "15",
	}
	// fmt.Println(partitions)

	sort.Strings(consumers)

	// fmt.Println(consumers)
	// fmt.Println("index", sort.SearchStrings(consumers, coord.id))
	j := sort.SearchStrings(consumers, coord.id)
	// TODO 1 get all consumers in the group

	for _, topic := range topics {
		fmt.Println(topic)

		n := len(partitions) / len(consumers)
		// fmt.Println("n", len(partitions)/len(consumers))

		// TODO add to zk
		fmt.Println("from", n*j)
		fmt.Println("to", n*(j+1))

		coord.assigned[topic] = []string{}
		for i := n * j; i != n*(j+1); i++ {
			if err := coord.sync.AddNode("/partition/"+group+"/"+topic+"/"+strconv.Itoa(i), []byte(coord.id), true); err != nil {
				if err == zk.ErrNodeExists {
					// TODO sleep and retry
				}

				coord.assigned[topic] = append(coord.assigned[topic], strconv.Itoa(i))
			}
		}

		// for _, partition := range coord.assigned {
		// // TODO remove
		// }

		// TODO if already taken , sleep and rebalance again

		// TODO assign range and take ownership
	}

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
		chGroup, err := coord.sync.WatchRegistry("/group/" + group)
		if err != nil {
			// TODO(AD)
		}
		chTopic, err := coord.sync.WatchRegistry("/topic/" + group)
		if err != nil {
			// TODO(AD)
		}

		select {
		case <-chGroup:
			coord.updated <- true
		case <-chTopic:
			coord.updated <- true
		}
	}
}
