package consumer

import (
	"net"

	"github.com/dunstall/wombat/client/pkg/record"
)

// TODO(AD) Set timeout on socket
type connection struct {
	sock net.Conn
}

func connect(addr string) (connection, error) {
	sock, err := net.Dial("tcp", addr)
	return connection{sock}, err
}

func (c *connection) send(request record.ConsumeRequest) error {
	_, err := c.sock.Write(request.Encode())
	return err
}

func (c *connection) receive() (record.ConsumeRecord, error) {
	return record.ReadConsumeRecord(c.sock)
}
