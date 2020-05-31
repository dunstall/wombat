package producer

import (
	"hash/crc32"
)

const (
	N_PARTITIONS = 15
)

type loadBalancer struct {
	next map[string]uint32
}

func newLoadBalancer() loadBalancer {
	return loadBalancer{
		next: make(map[string]uint32),
	}
}

func (lb *loadBalancer) nextPartition(topic string) uint32 {
	if n, ok := lb.next[topic]; ok {
		lb.next[topic] = n%N_PARTITIONS + 1
	} else {
		lb.next[topic] = 1
	}
	return lb.next[topic]
}

func (lb *loadBalancer) partitonFromKey(key []byte) uint32 {
	return crc32.Checksum(key, crc32.IEEETable) % N_PARTITIONS
}
