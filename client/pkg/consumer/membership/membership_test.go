package membership

import (
	"encoding/binary"
	"fmt"
	"path"
	"reflect"
	"sort"
	"strconv"
	"testing"

	"github.com/dunstall/wombat/client/pkg/consumer/registry"
	"github.com/dunstall/wombat/client/pkg/consumer/registry/mock_registry"
	"github.com/golang/mock/gomock"
	"github.com/samuel/go-zookeeper/zk"
)

func TestNewUniqueID(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	r := mock_registry.NewMockRegistry(ctrl)

	group := "mygroup"
	id := "e8d38de8-3794-45b1-8a37-f2b2c133268c"

	r.EXPECT().Create(
		gomock.Eq("/group/"+group+"/"+id),
		gomock.Eq([]byte{}),
		gomock.Eq(true),
	).Return(nil)

	m, err := New(group, id, r)
	if err != nil {
		t.Error(err)
	}

	if len(m.Assigned()) != 0 {
		t.Errorf("m.Assigned() = %v, expected %v", m.Assigned(), []string{})
	}
}

func TestNewIDAlreadyInUse(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	r := mock_registry.NewMockRegistry(ctrl)

	group := "mygroup"
	id := "e8d38de8-3794-45b1-8a37-f2b2c133268c"

	// Return an error as ID in use.
	r.EXPECT().Create(
		gomock.Eq("/group/"+group+"/"+id),
		gomock.Eq([]byte{}),
		gomock.Eq(true),
	).Return(registry.ErrNodeExists)

	_, err := New(group, id, r)
	if err != registry.ErrNodeExists {
		t.Errorf("expected node exists error")
	}
}

func TestFirstRebalance(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	r := mock_registry.NewMockRegistry(ctrl)
	// Ignore the node registering itself (tested above).
	r.EXPECT().Create(gomock.Any(), gomock.Any(), gomock.Any()).Return(nil)

	group := "mygroup"
	id := "id-2"
	m, err := New(group, id, r)
	if err != nil {
		t.Error(err)
	}

	topics := []string{"topic3", "topic2", "topic1"}
	consumers := []string{"id-3", id, "id-1"}

	r.EXPECT().GetRoot("/partition/"+group).Return(topics, nil)
	r.EXPECT().GetRoot("/group/"+group).Return(consumers, nil)

	expectedAssigned := []Chunk{}
	sort.Strings(topics)
	for _, t := range topics {
		for _, p := range partitionRange(id, consumers, nPartitions) {
			expectedAssigned = append(expectedAssigned, Chunk{t, p})
		}
	}

	for _, c := range expectedAssigned {
		p := path.Join("/", "partition", group, c.Topic, strconv.Itoa(int(c.Partition)))
		// Returns nil for success.
		r.EXPECT().Create(p, []byte(id), true).Return(nil)
	}

	if err = m.Rebalance(); err != nil {
		t.Error(err)
	}

	if !reflect.DeepEqual(expectedAssigned, m.Assigned()) {
		t.Errorf("m.Assigned() = %v, expected %v", m.Assigned(), expectedAssigned)
	}
}

func TestRebalanceAfterTopicUpdate(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	r := mock_registry.NewMockRegistry(ctrl)
	// Ignore the node registering itself (tested above).
	r.EXPECT().Create(gomock.Any(), gomock.Any(), gomock.Any()).Return(nil)

	group := "mygroup"
	id := "id-2"
	m, err := New(group, id, r)
	if err != nil {
		t.Error(err)
	}

	// Just assign expected initial chunks - this is tested above.
	topicsInitial := []string{"topic3", "topic2", "topic1"}
	sort.Strings(topicsInitial)
	initialAssigned := []Chunk{}
	consumers := []string{"id-3", id, "id-1"}
	for _, t := range topicsInitial {
		for _, p := range partitionRange(id, consumers, nPartitions) {
			initialAssigned = append(initialAssigned, Chunk{t, p})
		}
	}
	m.assigned = initialAssigned

	// Expect the rebalance to first clear all assigned partitions.
	for _, c := range initialAssigned {
		// Returns nil for success.
		p := "/partition/" + group + "/" + c.Topic + "/" + strconv.Itoa(int(c.Partition))
		r.EXPECT().Delete(p).Return(nil)
	}

	topicsNew := []string{"topic3", "topic2", "topic1", "topic-new1", "topic-new2"}

	r.EXPECT().GetRoot("/partition/"+group).Return(topicsNew, nil)
	r.EXPECT().GetRoot("/group/"+group).Return(consumers, nil)

	expectedAssigned := []Chunk{}
	sort.Strings(topicsNew)
	for _, t := range topicsNew {
		for _, p := range partitionRange(id, consumers, nPartitions) {
			expectedAssigned = append(expectedAssigned, Chunk{t, p})
		}
	}

	for _, c := range expectedAssigned {
		// Returns nil for success.
		p := "/partition/" + group + "/" + c.Topic + "/" + strconv.Itoa(int(c.Partition))
		r.EXPECT().Create(p, []byte(id), true).Return(nil)
	}

	if err = m.Rebalance(); err != nil {
		t.Error(err)
	}

	if !reflect.DeepEqual(expectedAssigned, m.Assigned()) {
		t.Errorf("m.Assigned() = %v, expected %v", m.Assigned(), expectedAssigned)
	}
}

func TestRebalanceAfterConsumerUpdate(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	r := mock_registry.NewMockRegistry(ctrl)
	// Ignore the node registering itself (tested above).
	r.EXPECT().Create(gomock.Any(), gomock.Any(), gomock.Any()).Return(nil)

	group := "mygroup"
	id := "id-2"
	m, err := New(group, id, r)
	if err != nil {
		t.Error(err)
	}

	// Just assign expected initial chunks - this is tested above.
	topicsInitial := []string{"topic3", "topic2", "topic1"}
	sort.Strings(topicsInitial)
	initialAssigned := []Chunk{}
	consumers := []string{"id-3", id, "id-1"}
	for _, t := range topicsInitial {
		for _, p := range partitionRange(id, consumers, nPartitions) {
			initialAssigned = append(initialAssigned, Chunk{t, p})
		}
	}
	m.assigned = initialAssigned

	// Expect the rebalance to first clear all assigned partitions.
	for _, c := range initialAssigned {
		// Returns nil for success.
		p := "/partition/" + group + "/" + c.Topic + "/" + strconv.Itoa(int(c.Partition))
		r.EXPECT().Delete(p).Return(nil)
	}

	topics := []string{"topic3", "topic2", "topic1"}
	consumersNew := []string{"id-3", id, "id-1", "id-new1", "id-new2"}

	r.EXPECT().GetRoot("/partition/"+group).Return(topics, nil)
	r.EXPECT().GetRoot("/group/"+group).Return(consumersNew, nil)

	sort.Strings(topics)

	expectedAssigned := []Chunk{}
	for _, t := range topics {
		for _, p := range partitionRange(id, consumersNew, nPartitions) {
			expectedAssigned = append(expectedAssigned, Chunk{t, p})
		}
	}

	for _, c := range expectedAssigned {
		// Returns nil for success.
		p := "/partition/" + group + "/" + c.Topic + "/" + strconv.Itoa(int(c.Partition))
		r.EXPECT().Create(p, []byte(id), true).Return(nil)
	}

	if err = m.Rebalance(); err != nil {
		t.Error(err)
	}

	if !reflect.DeepEqual(expectedAssigned, m.Assigned()) {
		t.Errorf("m.Assigned() = %v, expected %v", m.Assigned(), expectedAssigned)
	}
}

func TestAddTopicOk(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	r := mock_registry.NewMockRegistry(ctrl)
	r.EXPECT().Create(gomock.Any(), gomock.Any(), gomock.Any()).Return(nil)

	group := "mygroup"
	m, err := New(group, "", r)
	if err != nil {
		t.Error(err)
	}

	topic := "mytopic"
	r.EXPECT().CreateRoot(gomock.Eq("/partition/" + group + "/" + topic)).Return(nil)

	if m.AddTopic(topic) != nil {
		t.Error(err)
	}
}

func TestAddTopicErr(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	r := mock_registry.NewMockRegistry(ctrl)
	r.EXPECT().Create(gomock.Any(), gomock.Any(), gomock.Any()).Return(nil)

	group := "mygroup"
	m, err := New(group, "", r)
	if err != nil {
		t.Error(err)
	}

	topic := "mytopic"
	r.EXPECT().CreateRoot(
		gomock.Eq("/partition/" + group + "/" + topic),
	).Return(fmt.Errorf("dummy"))

	if m.AddTopic(topic) == nil {
		t.Error("expected error")
	}
}

func TestGetOffsetExists(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	r := mock_registry.NewMockRegistry(ctrl)
	r.EXPECT().Create(gomock.Any(), gomock.Any(), gomock.Any()).Return(nil)

	var offset uint64 = 2340824
	b := make([]byte, 8)
	binary.BigEndian.PutUint64(b, offset)

	r.EXPECT().Get(gomock.Eq("/offset/mygroup/mytopic/5")).Return(b, nil)

	group := "mygroup"
	m, err := New(group, "ae51b259-c015-465a-9acb-ca602ff044d7", r)
	if err != nil {
		t.Error(err)
	}

	ret, err := m.GetOffset(Chunk{"mytopic", 5})
	if err != nil {
		t.Error(err)
	}

	if ret != offset {
		t.Errorf("m.GetOffset(...) = %d, expected %d", ret, offset)
	}
}

func TestGetOffsetNotExists(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	r := mock_registry.NewMockRegistry(ctrl)
	r.EXPECT().Create(gomock.Any(), gomock.Any(), gomock.Any()).Return(nil)

	r.EXPECT().Get(gomock.Eq("/offset/mygroup/mytopic/5")).Return([]byte{}, zk.ErrNoNode)

	group := "mygroup"
	m, err := New(group, "ae51b259-c015-465a-9acb-ca602ff044d7", r)
	if err != nil {
		t.Error(err)
	}

	ret, err := m.GetOffset(Chunk{"mytopic", 5})
	if err != nil {
		t.Error(err)
	}

	if ret != 0 {
		t.Errorf("m.GetOffset(...) = %d, expected %d", ret, 0)
	}
}

func TestCommitOffset(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	r := mock_registry.NewMockRegistry(ctrl)
	r.EXPECT().Create(gomock.Any(), gomock.Any(), gomock.Any()).Return(nil)

	var offset uint64 = 2340824
	b := make([]byte, 8)
	binary.BigEndian.PutUint64(b, offset)

	r.EXPECT().Set(gomock.Eq("/offset/mygroup/mytopic/5"), gomock.Eq(b), false)

	group := "mygroup"
	m, err := New(group, "ae51b259-c015-465a-9acb-ca602ff044d7", r)
	if err != nil {
		t.Error(err)
	}

	if m.CommitOffset(offset, Chunk{"mytopic", 5}) != nil {
		t.Error(err)
	}
}

func TestPartitionRange(t *testing.T) {
	tests := []struct {
		id          string
		consumer    []string
		nPartitions int
		expected    []uint32
	}{
		{"1", []string{"1"}, 1, []uint32{1}},
		{"1", []string{"1", "2"}, 2, []uint32{1}},
		{"1", []string{"1", "2"}, 3, []uint32{1, 3}},
		{"2", []string{"1", "2"}, 3, []uint32{2}},
		{"1", []string{"1", "2", "3"}, 7, []uint32{1, 4, 7}},
		{"2", []string{"1", "2", "3"}, 7, []uint32{2, 5}},
		{"3", []string{"1", "2", "3"}, 7, []uint32{3, 6}},
	}
	for _, test := range tests {
		res := partitionRange(test.id, test.consumer, test.nPartitions)
		if !reflect.DeepEqual(res, test.expected) {
			t.Errorf("partitionRange(%s, %v, %d) = %v, expected %v", test.id, test.consumer, test.nPartitions, res, test.expected)
		}
	}
}
