package producer

import (
	"testing"
)

func TestNext(t *testing.T) {
	lb := newLoadBalancer()
	for i := 0; i <= 100; i++ {
		partition := lb.nextPartition("mytopic")
		expected := uint32(i%N_PARTITIONS + 1)
		if partition != expected {
			t.Errorf("lb.nextPartition() = %d, expected %d", partition, expected)
		}
	}
}

func TestPartitionFromKey(t *testing.T) {
	lb := newLoadBalancer()

	a := []struct {
		key      []byte
		expected uint32
	}{
		{[]byte("test"), 1},
		{[]byte("test123"), 13},
		{[]byte("test123-key"), 14},
	}

	for _, test := range a {
		partition := lb.partitonFromKey(test.key)
		if partition != test.expected {
			t.Errorf("lb.partitonFromKey(%s) = %d, expected %d", test.key, partition, test.expected)
		}
	}
}
