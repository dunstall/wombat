package consumer

import (
	"errors"
	"fmt"
	"time"

	"github.com/golang/glog"
	"github.com/samuel/go-zookeeper/zk"
)

type EventType int32

// Event represents a notification from the configuration service.
type Event struct {
	Type EventType
	Path string
}

const (
	// JoinedSession indicates the node connected and joined a session.
	JoinedSession EventType = 1
	// ConnectionLost indicates the nodes connection was dropped.
	ConnectionLost EventType = 2
	// SessionExpired indicates the config service expired the nodes current
	// session while it was disconnected.
	SessionExpired EventType = 3
)

var (
	ErrNotFound = errors.New("node not found")
)

type ZooKeeper struct {
	conn     *zk.Conn
	zkEvents <-chan zk.Event

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
func NewZooKeeper(servers []string, sessionTimeout time.Duration) (*ZooKeeper, error) {
	// This does not actually connect to the server - the actual connection is a
	// background process. Only returns an error if the config is bad.
	conn, zkEvents, err := zk.Connect(servers, sessionTimeout)
	if err != nil {
		glog.Errorf("bad server config: %s", err)
		return nil, fmt.Errorf("bad server config: %s", err)
	}

	cfg := ZooKeeper{
		conn:     conn,
		zkEvents: zkEvents,
		events:   make(chan Event),
		done:     make(chan interface{}),
	}
	go cfg.manage()

	return &cfg, nil
}

// Events returns a channel of events about the ZooKeeper connection.
// TODO Must handle
func (cfg *ZooKeeper) Events() <-chan Event {
	return cfg.events
}

// TODO(AD) Add flag for if allowed to already exist.
func (cfg *ZooKeeper) AddNode(path string, val []byte, isEphemeral bool) error {
	var flags int32 = 0
	if isEphemeral {
		flags = zk.FlagEphemeral
	}
	_, err := cfg.conn.Create(
		path,
		val,
		flags,
		zk.WorldACL(zk.PermRead|zk.PermWrite),
	)
	if err != nil {
		glog.Errorf("zookeeper add node error: %s: %s", path, err)
		return err
	}
	glog.Infof("added node %s -> %s (ephemeral = %t)", path, val, isEphemeral)
	return nil
}

func (cfg *ZooKeeper) GetNode(path string) ([]byte, error) {
	b, _, err := cfg.conn.Get(path)
	if err != nil {
		glog.Errorf("zookeeper get node error: %s: %s", path, err)
		return b, err
	}
	glog.Infof("get node %s -> %s", path, b)
	return b, nil
}

func (cfg *ZooKeeper) SetNode(path string, val []byte, isEphemeral bool) error {
	_, err := cfg.conn.Set(path, val, -1)
	if err == zk.ErrNoNode {
		return cfg.AddNode(path, val, isEphemeral)
	}
	if err != nil {
		glog.Errorf("zookeeper set node error: %s: %s", path, err)
		return err
	}
	glog.Infof("set node %s -> %s (ephemeral = %t)", path, val, isEphemeral)
	return nil
}

// TODO(AD) Add whole path. eg /a/b/c -> add /a then /a/b then /a/b/c
func (cfg *ZooKeeper) AddRegistry(path string) error {
	// Add empty znode for nodes root.
	_, err := cfg.conn.Create(
		path,
		[]byte{},
		0,
		zk.WorldACL(zk.PermRead|zk.PermWrite|zk.PermCreate|zk.PermDelete),
	)
	// Ignore if the root has been created by another node.
	if err != nil && err != zk.ErrNodeExists {
		glog.Errorf("zookeeper add root error: %s: %s", path, err)
		return err
	}
	glog.Infof("added root %s", path)
	return nil
}

func (cfg *ZooKeeper) WatchRegistry(path string) (<-chan zk.Event, error) {
	glog.Infof("zookeeper watching %s", path)
	_, _, ch, err := cfg.conn.ChildrenW(path)
	return ch, err
}

func (cfg *ZooKeeper) Close() {
	cfg.done <- true
	if cfg.conn != nil {
		cfg.conn.Close()
	}
	glog.Info("zookeeper closed")
}

func (cfg *ZooKeeper) manage() {
	for {
		select {
		case evt := <-cfg.zkEvents:
			cfg.handleZKEvent(evt)
		case <-cfg.done:
			return
		}
	}
}

func (cfg *ZooKeeper) handleZKEvent(evt zk.Event) {
	glog.Info("zookeeper connection event", evt)

	if evt.Type == zk.EventSession {
		cfg.handleEventSession(evt)
	}
}

func (cfg *ZooKeeper) handleEventSession(evt zk.Event) {
	switch evt.State {
	case zk.StateHasSession:
		glog.Info("zookeeper session created")
		cfg.events <- Event{Type: JoinedSession}
	case zk.StateExpired:
		glog.Info("zookeeper session expired")
		cfg.events <- Event{Type: SessionExpired}
	default:
		// TODO(AD) for now assume all other events indicate session lost
		glog.Info("zookeeper session lost")
		cfg.events <- Event{Type: ConnectionLost}
	}
}
