package producer

import (
	"net"

	"github.com/dunstall/wombat/client/pkg/record"
)

type connection struct {
	sock net.Conn
}

func connect(addr string) (connection, error) {
	sock, err := net.Dial("tcp", addr)
	return connection{sock}, err
}

func (c *connection) send(record record.ProduceRecord) error {
	_, err := c.sock.Write(record.Encode())
	return err
}

func (c *connection) close() error {
	return c.sock.Close()
}
