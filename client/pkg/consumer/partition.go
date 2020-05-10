package consumer

import (
	"strconv"
)

type Partition struct {
	Topic string
	N     uint32
}

func (p Partition) String() string {
	return p.Topic + "-" + strconv.Itoa(int(p.N))
}
