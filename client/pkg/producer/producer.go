package producer

import (
	"github.com/dunstall/wombatclient/pkg/record"
)

type Producer struct {
	lb   loadBalancer
	conn connection
}

func New(addr string) (Producer, error) {
	conn, err := connect(addr)
	if err != nil {
		return Producer{}, err
	}
	return Producer{
		lb:   newLoadBalancer(),
		conn: conn,
	}, nil
}

func (p *Producer) Send(record record.ProduceRecord) error {
	p.updatePartition(&record)

	// TODO(AD) For now have a single connection. Actually need one per
	// partition/topic for broker distribution.

	// TODO(AD) Ack
	return p.conn.send(record)
}

func (p *Producer) Close() error {
	return p.conn.close()
}

func (p *Producer) updatePartition(record *record.ProduceRecord) {
	if record.Partition() == 0 {
		if len(record.Key()) == 0 {
			record.SetPartition(p.lb.nextPartition(record.Topic()))
		} else {
			record.SetPartition(p.lb.partitonFromKey(record.Key()))
		}
	}
}
