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

	// TODO(AD) Keep events internal
	events chan Event
	done   chan interface{}
}

// NewZooKeeper returns a new zookeeper client.
//
// sessionTimeout is time to reestablish connection before the session is
// invalidated.
//
// Returns an error only if the configuration is bad as the actual connection
// occurs in the background.
func NewZKRegistry(servers []string, sessionTimeout time.Duration) (Registry, error) {
	// This does not actually connect to the server - the actual connection is a
	// background process. Only returns an error if the config is bad.
	conn, zkEvents, err := zk.Connect(servers, sessionTimeout)
	if err != nil {
		glog.Errorf("bad server config: %s", err)
		return nil, fmt.Errorf("bad server config: %s", err)
	}

	r := ZKRegistry{
		conn:     conn,
		zkEvents: zkEvents,
		events:   make(chan Event),
		done:     make(chan interface{}),
	}
	go r.manage()

	return &r, nil
}

func (r *ZKRegistry) Create(p string, val []byte, isEphemeral bool) error {
	var flags int32 = 0
	if isEphemeral {
		flags = zk.FlagEphemeral
	}

	_, err := r.conn.Create(
		p,
		val,
		flags,
		zk.WorldACL(zk.PermRead|zk.PermWrite),
	)
	if err != nil {
		if err == zk.ErrNoNode {
			// If the root does not exist create it and try again.
			if err := r.CreateRoot(path.Dir(p)); err != nil {
				return err
			}
			return r.Create(p, val, isEphemeral)
		}

		glog.Errorf("zookeeper failed to create node: %s: %s", p, err)
		return err
	}
	glog.Infof("created node %s -> %s (ephemeral = %t)", p, val, isEphemeral)
	return nil
}

// TODO(AD) Map to generic errors for common errors eg ErrNotFound
func (r *ZKRegistry) Get(path string) ([]byte, error) {
	b, _, err := r.conn.Get(path)
	if err != nil {
		glog.Errorf("zookeeper failed to get node: %s: %s", path, err)
		return b, err
	}
	glog.Infof("zookeeper gotten node %s -> %s", path, b)
	return b, nil
}

func (r *ZKRegistry) GetRoot(path string) ([]string, error) {
	c, _, err := r.conn.Children(path)
	if err != nil {
		glog.Errorf("zookeeper failed to get root: %s: %s", path, err)
	}
	glog.Infof("zookeeper gotten root %s -> %v", path, c)
	return c, err
}

func (r *ZKRegistry) Delete(path string) error {
	if err := r.conn.Delete(path, -1); err != nil {
		glog.Errorf("zookeeper failed to delete node: %s: %s", path, err)
		return err
	}
	glog.Infof("zookeeper deleted node %s", path)
	return nil
}

func (r *ZKRegistry) Watch(root string) (<-chan bool, error) {
	// TODO(AD)
	return make(chan bool), nil
}

// Events returns a channel of events about the ZooKeeper connection.
func (r *ZKRegistry) ConnEvents() <-chan Event {
	return r.events
}

func (r *ZKRegistry) Close() {
	/*  r.done <- true TODO */
	r.conn.Close()
	glog.Info("zookeeper closed")
}

func (r *ZKRegistry) manage() {
	for {
		select {
		case evt := <-r.zkEvents:
			r.handleEvent(evt)
		case <-r.done:
			return
		}
	}
}

func (r *ZKRegistry) handleEvent(evt zk.Event) {
	glog.Info("zookeeper connection event", evt)

	if evt.Type == zk.EventSession {
		r.handleEventSession(evt)
	}
}

func (r *ZKRegistry) handleEventSession(evt zk.Event) {
	switch evt.State {
	case zk.StateHasSession:
		glog.Info("zookeeper session created")
		r.events <- Event{Type: JoinedSession}
	case zk.StateExpired:
		glog.Info("zookeeper session expired")
		r.events <- Event{Type: SessionExpired}
	default:
		// TODO(AD) for now assume all other events indicate session lost
		glog.Info("zookeeper session lost")
		r.events <- Event{Type: ConnectionLost}
	}
}

func (r *ZKRegistry) CreateRoot(p string) error {
	if p == "/" {
		return nil
	}

	if err := r.CreateRoot(path.Dir(p)); err != nil {
		return err
	}

	_, err := r.conn.Create(
		p,
		[]byte{},
		0,
		zk.WorldACL(zk.PermRead|zk.PermWrite|zk.PermCreate|zk.PermDelete),
	)
	// Ignore if the root has been created by another node.
	if err != nil && err != zk.ErrNodeExists {
		glog.Errorf("zookeeper add root error: %s: %s", p, err)
		return err
	}
	glog.Infof("added root %s", p)
	return nil
}
