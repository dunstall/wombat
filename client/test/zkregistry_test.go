// +build zk

package tests

import (
	"path"
	"reflect"
	"testing"
	"time"

	"github.com/dunstall/wombatclient/pkg/consumer/registry"
	"github.com/google/uuid"
	"github.com/samuel/go-zookeeper/zk"
)

var (
	addrs = []string{"192.168.48.2", "192.168.48.3", "192.168.48.4"}
)

func TestCreateEphemeralNode(t *testing.T) {
	r, err := registry.NewZKRegistry(addrs, time.Second*1)
	if err != nil {
		t.Fatal(err)
	}

	p := path.Join("/", uuid.New().String(), uuid.New().String(), uuid.New().String())
	val := uuid.New().String()
	if err = r.Create(p, []byte(val), true); err != nil {
		t.Fatal(err)
	}

	b, err := r.Get(p)
	if err != nil {
		t.Fatal(err)
	}
	if string(b) != val {
		t.Errorf("r.Get(...) = %v, expected %v", b, []byte(val))
	}

	// Close the connectino and sleep so the session expires- this should remove
	// the created ephemeral node.
	r.Close()
	time.Sleep(time.Second * 3)

	r, err = registry.NewZKRegistry(addrs, time.Second*5)
	if err != nil {
		t.Fatal(err)
	}
	defer r.Close()

	_, err = r.Get(p)
	if err != zk.ErrNoNode {
		t.Fatal("expected ErrNoNode")
	}
}

func TestCreateNonEphemeralNodeAndDelete(t *testing.T) {
	r, err := registry.NewZKRegistry(addrs, time.Second*1)
	if err != nil {
		t.Fatal(err)
	}

	p := path.Join("/", uuid.New().String(), uuid.New().String(), uuid.New().String())
	val := uuid.New().String()
	if err = r.Create(p, []byte(val), false); err != nil {
		t.Fatal(err)
	}

	// Close the connectino and sleep so the session expires. As the node is not
	// ephemeral it should not be removed.
	r.Close()
	time.Sleep(time.Second * 3)

	r, err = registry.NewZKRegistry(addrs, time.Second*5)
	if err != nil {
		t.Fatal(err)
	}
	defer r.Close()

	b, err := r.Get(p)
	if err != nil {
		t.Fatal(err)
	}
	if string(b) != val {
		t.Errorf("r.Get(...) = %v, expected %v", b, []byte(val))
	}

	if err := r.Delete(p); err != nil {
		t.Fatal(err)
	}
	_, err = r.Get(p)
	if err != zk.ErrNoNode {
		t.Fatal("expected ErrNoNode")
	}
}

func TestGetRoot(t *testing.T) {
	r, err := registry.NewZKRegistry(addrs, time.Second*1)
	if err != nil {
		t.Fatal(err)
	}

	root := path.Join("/", uuid.New().String(), uuid.New().String(), uuid.New().String())
	nodes := make(map[string]bool)
	for i := 0; i != 50; i++ {
		node := uuid.New().String()
		nodes[node] = true

		if err = r.Create(root+"/"+node, []byte{}, true); err != nil {
			t.Fatal(err)
		}
	}

	ret, err := r.GetRoot(root)
	if err != nil {
		t.Fatal(err)
	}
	a := make(map[string]bool)
	for _, b := range ret {
		a[b] = true
	}

	if !reflect.DeepEqual(a, nodes) {
		t.Fatal("err nodes")
	}
}

func TestSet(t *testing.T) {
	r, err := registry.NewZKRegistry(addrs, time.Second*1)
	if err != nil {
		t.Fatal(err)
	}
	defer r.Close()

	p := path.Join("/", uuid.New().String(), uuid.New().String(), uuid.New().String())
	val := uuid.New().String()
	if err = r.Set(p, []byte(val), true); err != nil {
		t.Fatal(err)
	}

	b, err := r.Get(p)
	if err != nil {
		t.Fatal(err)
	}
	if string(b) != val {
		t.Errorf("r.Get(...) = %v, expected %v", b, []byte(val))
	}

	val = uuid.New().String()
	if err = r.Set(p, []byte(val), true); err != nil {
		t.Fatal(err)
	}

	b, err = r.Get(p)
	if err != nil {
		t.Fatal(err)
	}
	if string(b) != val {
		t.Errorf("r.Get(...) = %v, expected %v", b, []byte(val))
	}
}
