package consumer

import (
	"github.com/dunstall/wombatclient/pkg/record"
)

type Consumer struct {
	conn connection
}

func New(addr string) (Consumer, error) {
	conn, err := connect(addr)
	if err != nil {
		return Consumer{}, err
	}
	return Consumer{
		conn: conn,
	}, nil
}

func (c *Consumer) Poll(request record.ConsumeRequest) (record.ConsumeRecord, error) {
	// TODO(AD) Request in background on poll - should assign partitions with client
	// coordination.

	if err := c.conn.send(request); err != nil {
		return record.ConsumeRecord{}, err
	}

	return c.conn.receive()
}
