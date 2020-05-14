package membership

import (
	"fmt"
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

// TODO(AD) Rebalance first
// func TestInitialRebalance(t *testing.T) {
// }

// TODO(AD) Rebalance not first

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
	r.EXPECT().Create(
		gomock.Eq("/partition/"+group+"/"+topic),
		gomock.Eq([]byte{}),
		gomock.Eq(false),
	).Return(nil)

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
	r.EXPECT().Create(
		gomock.Eq("/partition/"+group+"/"+topic),
		gomock.Eq([]byte{}),
		gomock.Eq(false),
	).Return(fmt.Errorf("dummy"))

	if m.AddTopic(topic) == nil {
		t.Error("expected error")
	}
}

// TODO(AD) RequiresRebalance
