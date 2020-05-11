package consumer

import (
	"fmt"
	"time"

	"github.com/google/uuid"
	"github.com/samuel/go-zookeeper/zk"
)

const (
	consumerRegistryPath = "/consumer"
)

type consumerRegistry struct {
	// TODO(AD) Multiple zk connections - when rewriting make this shared abstraction
	zkConn *zk.Conn
	id     string
}

func join(servers []string, sessionTimeout time.Duration, consumerGroup string) (consumerRegistry, error) {
	zkConn, _, err := zk.Connect(servers, sessionTimeout)
	if err != nil {
		return consumerRegistry{}, fmt.Errorf("bad server config: %s", err)
	}

	r := consumerRegistry{zkConn, uuid.New().String()}
	if err = r.addRegistry(); err != nil {
		return consumerRegistry{}, fmt.Errorf("failed to create registry: %s", err)
	}

	// TODO if exists must create new ID?
	fmt.Println("create at", r.nodePath())
	_, err = r.zkConn.Create(
		r.nodePath(), []byte(consumerGroup), zk.FlagEphemeral, zk.WorldACL(zk.PermRead|zk.PermWrite),
	)
	return r, err
}

func (r *consumerRegistry) addRegistry() error { // TODO(AD) Dup
	_, err := r.zkConn.Create(
		consumerRegistryPath,
		[]byte{},
		0,
		zk.WorldACL(zk.PermRead|zk.PermWrite|zk.PermCreate|zk.PermDelete),
	)
	// Ignore if the root has been created by another node.
	if err != nil && err != zk.ErrNodeExists {
		return err
	}
	return nil
}

func (r *consumerRegistry) nodePath() string {
	return consumerRegistryPath + "/" + r.id
}
