package conf

import (
	"fmt"
	"reflect"
	"strings"
	"testing"
	"time"
)

func TestParseGroup(t *testing.T) {
	group := "mywombatgroup"
	data := []byte(fmt.Sprintf(`
    [consumer]
    group=%s
  `, group))

	conf, err := ParseConf(data)
	if err != nil {
		t.Error(err)
	}

	if conf.Group() != group {
		t.Errorf("conf.Group() = %s, expected %s", conf.Group(), group)
	}
}

func TestParseBroker(t *testing.T) {
	broker := "34.21.08.2:3110"
	data := []byte(fmt.Sprintf(`
    [consumer]
    broker=%s
  `, broker))

	conf, err := ParseConf(data)
	if err != nil {
		t.Error(err)
	}

	if conf.Broker() != broker {
		t.Errorf("conf.Broker() = %s, expected %s", conf.Broker(), broker)
	}
}

func TestParseZooKeeper(t *testing.T) {
	zk := []string{"34.21.08.2:2180", "84.21.13.5:324", "27.2.5.27:4211"}
	data := []byte(fmt.Sprintf(`
    [consumer]
    zookeeper=%s
  `, strings.Join(zk, ", ")))

	conf, err := ParseConf(data)
	if err != nil {
		t.Error(err)
	}

	if !reflect.DeepEqual(conf.ZooKeeper(), zk) {
		t.Errorf("conf.ZooKeeper() = %s, expected %s", conf.ZooKeeper(), zk)
	}
}

func TestParseZooTimeout(t *testing.T) {
	timeout := 1500
	data := []byte(fmt.Sprintf(`
    [consumer]
    timeout=%d
  `, timeout))

	conf, err := ParseConf(data)
	if err != nil {
		t.Error(err)
	}

	expected := time.Duration(timeout) * time.Millisecond
	if conf.Timeout() != expected {
		t.Errorf("conf.Timeout() = %v, expected %v", conf.Timeout(), expected)
	}
}

func TestBadConfig(t *testing.T) {
	_, err := ParseConf([]byte(`
    not really valid...
  `))
	if err == nil {
		t.Error("expected bad config err")
	}
}
