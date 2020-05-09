package consumer

import (
	"strconv"
)

type Partition struct {
	topic string
	n     uint32
}

func (p Partition) String() string {
	return p.topic + "-" + strconv.Itoa(int(p.n))
}
