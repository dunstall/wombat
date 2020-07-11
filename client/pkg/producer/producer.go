package producer

import (
	"net"

	"github.com/dunstall/wombatclient/pkg/record"
)

type Producer struct {
	conn net.Conn
}

func Connect(addr string) (Producer, error) {
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return Producer{}, err
	}
	return Producer{conn}, nil
}

func (p *Producer) Produce(partitionID uint32, payload []byte) bool {
	r, ok := record.NewRecord(payload)
	if !ok {
		return false
	}

	m, ok := record.NewMessage(record.ProduceRequest, partitionID, r.Encode())
	if !ok {
		return false
	}

	_, err := p.conn.Write(m.Encode())
	if err != nil {
		return false
	}

	return true
}

func (p *Producer) Close() {
	defer p.conn.Close()
}
