package membership

import (
	"fmt"
	"path"
	"reflect"
	"sort"
	"strconv"
	"testing"

	"github.com/dunstall/wombatclient/pkg/consumer/registry"
	"github.com/dunstall/wombatclient/pkg/consumer/registry/mock_registry"
	"github.com/golang/mock/gomock"
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

	// Should be assigned the middle third of partitions for each topic.
	var expectedFrom uint32 = 6
	var expectedTo uint32 = 11
	expectedAssigned := []Chunk{}
	sort.Strings(topics)
	for _, t := range topics {
		for p := expectedFrom; p != expectedTo; p++ {
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
	var from uint32 = 6
	var to uint32 = 11
	sort.Strings(topicsInitial)
	initialAssigned := []Chunk{}
	for _, t := range topicsInitial {
		for p := from; p != to; p++ {
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
	consumers := []string{"id-3", id, "id-1"}

	r.EXPECT().GetRoot("/partition/"+group).Return(topicsNew, nil)
	r.EXPECT().GetRoot("/group/"+group).Return(consumers, nil)

	var expectedFrom uint32 = 6
	var expectedTo uint32 = 11
	sort.Strings(topicsNew)
	expectedAssigned := []Chunk{}
	for _, t := range topicsNew {
		for p := expectedFrom; p != expectedTo; p++ {
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
	var from uint32 = 6
	var to uint32 = 11
	sort.Strings(topicsInitial)
	initialAssigned := []Chunk{}
	for _, t := range topicsInitial {
		for p := from; p != to; p++ {
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

	var expectedFrom uint32 = 4
	var expectedTo uint32 = 7
	sort.Strings(topics)
	expectedAssigned := []Chunk{}
	for _, t := range topics {
		for p := expectedFrom; p != expectedTo; p++ {
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

func TestInitialRebalanceNoTopics(t *testing.T) {
	ctrl := gomock.NewController(t)
	defer ctrl.Finish()
	r := mock_registry.NewMockRegistry(ctrl)
	r.EXPECT().Create(gomock.Any(), gomock.Any(), gomock.Any()).Return(nil)

	group := "mygroup"
	m, err := New(group, "", r)
	if err != nil {
		t.Error(err)
	}

	// Return no topics.
	r.EXPECT().GetRoot("/partition/"+group).Return([]string{}, nil)
	r.EXPECT().GetRoot("/group/"+group).Return([]string{"a", "b", "c"}, nil)

	if err = m.Rebalance(); err != nil {
		t.Error(err)
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
