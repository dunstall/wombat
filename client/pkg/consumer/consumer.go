package consumer

import (
	"io/ioutil"
	"os"

	"github.com/dunstall/wombatclient/pkg/consumer/conf"
	"github.com/dunstall/wombatclient/pkg/record"
)

type Consumer struct {
	conn    connection
	offsets offsets
	coord   coordinator
}

// TODO(AD) Add subscribe that adds topics to zk
func New(confPath string) (Consumer, error) {
	conf, err := loadConf(confPath)
	if err != nil {
		return Consumer{}, err
	}

	sync, err := NewZooKeeper(conf.ZooKeeper(), conf.Timeout())
	if err != nil {
		return Consumer{}, err
	}

	conn, err := connect(conf.Broker())
	if err != nil {
		return Consumer{}, err
	}

	offsets, err := newOffsets(sync, conf.Group())
	if err != nil {
		return Consumer{}, err
	}

	coord, err := newCoordinator(sync, conf.Group())
	if err != nil {
		return Consumer{}, err
	}

	return Consumer{
		conn,
		offsets,
		coord,
	}, nil
}

// TODO(AD) Poll should not take arguments - distribute partitions automatically
func (c *Consumer) Poll(partition Partition) (record.ConsumeRecord, error) {
	// TODO(AD) Request in background on poll - should assign partitions with client
	// coordination.

	select {
	case <-c.coord.updates():
		c.coord.rebalance()
	default:
	}

	offset, err := c.offsets.lookup(partition)
	if err != nil {
		return record.ConsumeRecord{}, err
	}

	request := record.NewConsumeRequest(partition.Topic, partition.N, offset)
	if err := c.conn.send(request); err != nil {
		return record.ConsumeRecord{}, err
	}
	return c.conn.receive()
}

func (c *Consumer) Commit(record record.ConsumeRecord, partition Partition) error {
	return c.offsets.commit(partition, record.NextOffset())
}

func loadConf(path string) (conf.Conf, error) {
	file, err := os.Open(path)
	if err != nil {
		return conf.Conf{}, err
	}
	defer file.Close()

	b, err := ioutil.ReadAll(file)
	if err != nil {
		return conf.Conf{}, err
	}

	return conf.ParseConf(b)
}
