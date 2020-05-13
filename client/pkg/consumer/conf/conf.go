package conf

import (
	"strings"
	"time"

	ini "gopkg.in/ini.v1"
)

type Conf struct {
	group     string
	broker    string
	zookeeper []string
	timeout   time.Duration
}

func ParseConf(data []byte) (Conf, error) {
	iniConf, err := ini.Load(data)
	if err != nil {
		return Conf{}, err
	}

	timeout, err := iniConf.Section("consumer").Key("timeout").Int()
	if err != nil {
		// TODO(AD) log bad timeout so using default
		timeout = 5000
	}

	return Conf{
		group:     iniConf.Section("consumer").Key("group").String(),
		broker:    iniConf.Section("consumer").Key("broker").String(),
		zookeeper: splitAndTrim(iniConf.Section("consumer").Key("zookeeper").String()),
		timeout:   time.Duration(timeout) * time.Millisecond,
	}, nil
}

func (conf *Conf) Group() string {
	return conf.group
}

func (conf *Conf) Broker() string {
	return conf.broker
}

func (conf *Conf) ZooKeeper() []string {
	return conf.zookeeper
}

func (conf *Conf) Timeout() time.Duration {
	return conf.timeout
}

func splitAndTrim(l string) []string {
	res := []string{}
	for _, s := range strings.Split(l, ",") {
		res = append(res, strings.TrimSpace(s))
	}
	return res
}
