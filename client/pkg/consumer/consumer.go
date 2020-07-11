package consumer

import (
	"io"
	"net"

	"github.com/dunstall/wombatclient/pkg/record"
)

type Consumer struct {
	conn net.Conn
}

func Connect(addr string) (Consumer, error) {
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return Consumer{}, err
	}
	return Consumer{conn}, nil
}

func (c *Consumer) Consume(partitionID uint32, offset uint32) ([]byte, bool) {
	rr := record.NewRecordRequest(offset)

	m, ok := record.NewMessage(record.ConsumeRequest, partitionID, rr.Encode())
	if !ok {
		return []byte{}, false
	}

	_, err := c.conn.Write(m.Encode())
	if err != nil {
		return []byte{}, false
	}

	b := make([]byte, 12)
	_, err = io.ReadFull(c.conn, b)
	if err != nil {
		return []byte{}, false
	}

	h, ok := record.DecodeMessageHeader(b)
	if !ok {
		return []byte{}, false
	}

	b = make([]byte, h.PayloadSize())
	_, err = io.ReadFull(c.conn, b)
	if err != nil {
		return []byte{}, false
	}

	r, ok := record.DecodeRecord(b)
	if !ok {
		return []byte{}, false
	}

	return r.Data(), true
}

func (c *Consumer) Close() {
	defer c.conn.Close()
}
