package consumer

import (
	"fmt"
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

func (c *Consumer) Stat(partitionID uint32) (uint32, bool) {
	m, ok := record.NewMessage(record.TypeStatRequest, partitionID, []byte{})
	if !ok {
		fmt.Println("failed to create message")
		return 0, false
	}

	_, err := c.conn.Write(m.Encode())
	if err != nil {
		fmt.Println("failed to write to connection")
		return 0, false
	}

	b := make([]byte, 12)
	_, err = io.ReadFull(c.conn, b)
	if err != nil {
		fmt.Println("failed to read from connection")
		return 0, false
	}

	h, ok := record.DecodeMessageHeader(b)
	if !ok {
		fmt.Println("failed to decode message header")
		return 0, false
	}

	b = make([]byte, h.PayloadSize())
	_, err = io.ReadFull(c.conn, b)
	if err != nil {
		fmt.Println("failed to read message payload")
		return 0, false
	}

	s, ok := record.DecodeStatResponse(b)
	if !ok {
		fmt.Println("failed to decode stat")
		return 0, false
	}

	return s.Size(), true
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
