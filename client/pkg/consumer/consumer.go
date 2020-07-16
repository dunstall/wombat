package consumer

import (
	"io/ioutil"
	"math/rand"
	"os"
	"time"

	"github.com/dunstall/wombatclient/pkg/consumer/conf"
	"github.com/dunstall/wombatclient/pkg/consumer/membership"
	"github.com/dunstall/wombatclient/pkg/consumer/registry"
	"github.com/dunstall/wombatclient/pkg/record"
	"github.com/google/uuid"
)

type Consumer struct {
	conn connection
	r    registry.Registry
	m    membership.Membership
}

func New(confPath string) (Consumer, error) {
	conf, err := loadConf(confPath)
	if err != nil {
		return Consumer{}, err
	}

	r, err := registry.NewZKRegistry(conf.ZooKeeper(), conf.Timeout())
	if err != nil {
		return Consumer{}, err
	}
	if err := r.Watch(path.Join("/", "partition", conf.Group())); err != nil { // TODO(AD)
		return Consumer{}, err
	}
	if err := r.Watch(path.Join("/", "group", conf.Group())); err != nil { // TODO(AD)
		return Consumer{}, err
	}

	// TODO(AD) must rebalance before poll

	id := uuid.New().String()
	m, err := membership.New(conf.Group(), id, r)
	if err != nil {
		return Consumer{}, err
	}

	conn, err := connect(conf.Broker())
	if err != nil {
		return Consumer{}, err
	}

	rand.Seed(time.Now().Unix())

	return Consumer{
		conn,
		r,
		m,
	}, nil
}

func (c *Consumer) Poll() (record.ConsumeRecord, membership.Chunk, error) {
	select {
	case <-c.r.Events(): // TODO(AD) Add watch (in membership?)
		if err := c.m.Rebalance(); err != nil { // TODO(AD) Sleep and retry on error
			return record.ConsumeRecord{}, membership.Chunk{}, err
		}
		// TODO(AD) Need to re-watch after every event
	default:
	}

	// TODO(AD) For now just select random partition to poll - should have
	// a thread per owned partition that pulls in background.
	chunk := c.m.Assigned()[rand.Intn(len(c.m.Assigned()))]

	offset, err := c.m.GetOffset(chunk)
	if err != nil {
		return record.ConsumeRecord{}, membership.Chunk{}, err
	}

	request := record.NewConsumeRequest(chunk.Topic, chunk.Partition, offset)
	if err := c.conn.send(request); err != nil {
		return record.ConsumeRecord{}, membership.Chunk{}, err
	}
	r, err := c.conn.receive() // TODO timeout
	if err != nil {
		return record.ConsumeRecord{}, membership.Chunk{}, err
	}

	return r, chunk, nil
}

func (c *Consumer) Commit(record record.ConsumeRecord, chunk membership.Chunk) error {
	return c.m.CommitOffset(record.NextOffset(), chunk)
}

func (c *Consumer) Subscribe(topic string) error {
	return c.m.AddTopic(topic)
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
