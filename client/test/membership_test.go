// +build membership

// Tests the consumer membership package. As this is already well unit-tested
// by mocking zookeeper only need a few simple tests to check the integration
// with zookeeper.

package tests

import (
	"reflect"
	"testing"
	"time"

	"github.com/dunstall/wombatclient/pkg/consumer/membership"
	"github.com/dunstall/wombatclient/pkg/consumer/registry"
	"github.com/google/uuid"
	"github.com/samuel/go-zookeeper/zk"
)

func TestSingleConsumerRebalanceTopics(t *testing.T) {
	addrs := []string{"192.168.48.2", "192.168.48.3", "192.168.48.4"}

	r, err := registry.NewZKRegistry(addrs, time.Second*1)
	if err != nil {
		t.Fatal(err)
	}

	group := uuid.New().String()

	m, err := membership.New(group, uuid.New().String(), r)
	if err != nil {
		t.Fatal(err)
	}

	if err = m.AddTopic("mytopic"); err != nil {
		t.Fatal(err)
	}
	if err = m.Rebalance(); err != nil {
		t.Fatal(err)
	}

	expectedAssigned := []membership.Chunk{}
	for p := 1; p != 16; p++ {
		expectedAssigned = append(expectedAssigned, membership.Chunk{"mytopic", uint32(p)})
	}

	if !reflect.DeepEqual(expectedAssigned, m.Assigned()) {
		t.Errorf("m.Assigned() = %v, expected %v", m.Assigned(), expectedAssigned)
	}

	// Add a new topic and rebalance.
	if err = m.AddTopic("mytopic-new"); err != nil {
		t.Fatal(err)
	}
	if err = m.Rebalance(); err != nil {
		t.Fatal(err)
	}

	for p := 1; p != 16; p++ {
		expectedAssigned = append(expectedAssigned, membership.Chunk{"mytopic-new", uint32(p)})
	}

	if !reflect.DeepEqual(expectedAssigned, m.Assigned()) {
		t.Errorf("m.Assigned() = %v, expected %v", m.Assigned(), expectedAssigned)
	}
}

func TestMultiConsumerRebalance(t *testing.T) {
	addrs := []string{"192.168.48.2", "192.168.48.3", "192.168.48.4"}

	r, err := registry.NewZKRegistry(addrs, time.Second*1)
	if err != nil {
		t.Fatal(err)
	}

	group := uuid.New().String()

	m1, err := membership.New(group, "c5544bc4-72f5-4ed7-9568-30d55d897ff8", r)
	if err != nil {
		t.Fatal(err)
	}

	if err = m1.AddTopic("mytopic"); err != nil {
		t.Fatal(err)
	}
	if err = m1.Rebalance(); err != nil {
		t.Fatal(err)
	}

	expectedAssigned := []membership.Chunk{}
	for p := 1; p != 16; p++ {
		expectedAssigned = append(expectedAssigned, membership.Chunk{"mytopic", uint32(p)})
	}

	if !reflect.DeepEqual(expectedAssigned, m1.Assigned()) {
		t.Errorf("m.Assigned() = %v, expected %v", m1.Assigned(), expectedAssigned)
	}

	m2, err := membership.New(group, "dfb65542-7c28-40f7-aaed-c7fc25df57bc", r)
	if err != nil {
		t.Fatal(err)
	}
	// Expect to fail as m1 has not releases its assigned partitions.
	if err = m2.Rebalance(); err != zk.ErrNodeExists {
		t.Fatal("expected node exists")
	}

	if err = m1.Rebalance(); err != nil {
		t.Fatal(err)
	}
	if err = m2.Rebalance(); err != nil {
		t.Fatal(err)
	}

	expectedAssignedM1 := []membership.Chunk{}
	expectedAssignedM1 = append(expectedAssignedM1, membership.Chunk{"mytopic", uint32(1)})
	expectedAssignedM1 = append(expectedAssignedM1, membership.Chunk{"mytopic", uint32(3)})
	expectedAssignedM1 = append(expectedAssignedM1, membership.Chunk{"mytopic", uint32(5)})
	expectedAssignedM1 = append(expectedAssignedM1, membership.Chunk{"mytopic", uint32(7)})
	expectedAssignedM1 = append(expectedAssignedM1, membership.Chunk{"mytopic", uint32(9)})
	expectedAssignedM1 = append(expectedAssignedM1, membership.Chunk{"mytopic", uint32(11)})
	expectedAssignedM1 = append(expectedAssignedM1, membership.Chunk{"mytopic", uint32(13)})
	expectedAssignedM1 = append(expectedAssignedM1, membership.Chunk{"mytopic", uint32(15)})
	if !reflect.DeepEqual(expectedAssignedM1, m1.Assigned()) {
		t.Errorf("m1.Assigned() = %v, expected %v", m1.Assigned(), expectedAssignedM1)
	}

	expectedAssignedM2 := []membership.Chunk{}
	expectedAssignedM2 = append(expectedAssignedM2, membership.Chunk{"mytopic", uint32(2)})
	expectedAssignedM2 = append(expectedAssignedM2, membership.Chunk{"mytopic", uint32(4)})
	expectedAssignedM2 = append(expectedAssignedM2, membership.Chunk{"mytopic", uint32(6)})
	expectedAssignedM2 = append(expectedAssignedM2, membership.Chunk{"mytopic", uint32(8)})
	expectedAssignedM2 = append(expectedAssignedM2, membership.Chunk{"mytopic", uint32(10)})
	expectedAssignedM2 = append(expectedAssignedM2, membership.Chunk{"mytopic", uint32(12)})
	expectedAssignedM2 = append(expectedAssignedM2, membership.Chunk{"mytopic", uint32(14)})
	if !reflect.DeepEqual(expectedAssignedM2, m2.Assigned()) {
		t.Errorf("m2.Assigned() = %v, expected %v", m2.Assigned(), expectedAssignedM2)
	}
}
