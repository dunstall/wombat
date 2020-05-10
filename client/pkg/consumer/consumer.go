package consumer

import (
	"fmt"
	"time"

	"github.com/dunstall/wombatclient/pkg/record"
)

type Consumer struct {
	conn    connection
	offsets offsets
}

func New(broker string, zkServers []string, sessionTimeout time.Duration) (Consumer, error) {
	conn, err := connect(broker)
	if err != nil {
		return Consumer{}, err
	}

	offsets, err := newOffsets(zkServers, sessionTimeout)
	if err != nil {
		return Consumer{}, err
	}

	return Consumer{
		conn,
		offsets,
	}, nil
}

// TODO(AD) Poll should not take arguments - distribute partitions automatically
func (c *Consumer) Poll(partition Partition) (record.ConsumeRecord, error) {
	// TODO(AD) Request in background on poll - should assign partitions with client
	// coordination.

	offset, err := c.offsets.lookup(partition)
	if err != nil {
		return record.ConsumeRecord{}, err
	}

	request := record.NewConsumeRequest(partition.topic, partition.n, offset)
	if err := c.conn.send(request); err != nil {
		return record.ConsumeRecord{}, err
	}
	return c.conn.receive()
}

func (c *Consumer) Commit(record record.ConsumeRecord, partition Partition) error {
	return c.offsets.commit(partition, record.NextOffset())
}
