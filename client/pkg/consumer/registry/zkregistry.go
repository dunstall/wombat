package registry

import (
	"fmt"
	"path"
	"time"

	"github.com/golang/glog"
	"github.com/samuel/go-zookeeper/zk"
)

type ZKRegistry struct {
	conn     *zk.Conn
	zkEvents <-chan zk.Event
}

// NewZKRegistry returns a new registry implemented with ZooKeeper.
//
// sessionTimeout is time to reestablish connection before the session is
// invalidated (causing all ephemeral nodes to be deleted).
func NewZKRegistry(bootstap []string, sessionTimeout time.Duration) (Registry, error) {
	conn, zkEvents, err := zk.Connect(bootstap, sessionTimeout)
	if err != nil {
		err = fmt.Errorf("failed to connect to server: %s", err)
		glog.Error(err)
		return nil, err
	}
	return &ZKRegistry{conn: conn, zkEvents: zkEvents}, nil
}

func (r *ZKRegistry) Create(p string, val []byte, isEphemeral bool) error {
	var flags int32 = 0
	if isEphemeral {
		flags = zk.FlagEphemeral
	}

	acl := zk.WorldACL(zk.PermRead | zk.PermWrite)
	_, err := r.conn.Create(p, val, flags, acl)
	if err != nil {
		if err == zk.ErrNoNode {
			// If the root does not exist create it and try again.
			if err := r.CreateRoot(path.Dir(p)); err != nil {
				return err
			}
			return r.Create(p, val, isEphemeral)
		}

		glog.Errorf("zk failed to create node: %s: %s", p, err)
		return err
	}
	glog.Infof("zk created node %s -> %s (ephemeral = %t)", p, val, isEphemeral)
	return nil
}

// CreateRoot recursively creates all roots in the path.
func (r *ZKRegistry) CreateRoot(p string) error {
	if p == "/" {
		return nil
	}

	if err := r.CreateRoot(path.Dir(p)); err != nil {
		return err
	}

	acl := zk.WorldACL(zk.PermRead | zk.PermWrite | zk.PermCreate | zk.PermDelete)
	_, err := r.conn.Create(p, []byte{}, 0, acl)
	// Ignore if the root has been created by another node.
	if err != nil && err != zk.ErrNodeExists {
		glog.Errorf("zk add root error: %s: %s", p, err)
		return err
	}
	glog.Infof("zk added root %s", p)
	return nil
}

func (r *ZKRegistry) Get(path string) ([]byte, error) {
	b, _, err := r.conn.Get(path)
	if err != nil {
		glog.Errorf("zk failed to get node: %s: %s", path, err)
		return b, err
	}
	glog.Infof("zk gotten node %s -> %s", path, b)
	return b, nil
}

func (r *ZKRegistry) GetRoot(path string) ([]string, error) {
	c, _, err := r.conn.Children(path)
	if err != nil {
		glog.Errorf("zk failed to get root: %s: %s", path, err)
	}
	glog.Infof("zk gotten root %s -> %v", path, c)
	return c, err
}

func (r *ZKRegistry) Delete(path string) error {
	if err := r.conn.Delete(path, -1); err != nil {
		glog.Errorf("zk failed to delete node: %s: %s", path, err)
		return err
	}
	glog.Infof("zk deleted node %s", path)
	return nil
}

func (r *ZKRegistry) Watch(root string) (<-chan bool, error) {
	// TODO(AD)
	return make(chan bool), nil
}

func (r *ZKRegistry) Close() {
	r.conn.Close()
	glog.Info("zk closed")
}
