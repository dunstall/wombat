package test

import (
	"testing"

	"github.com/dunstall/wombatclient/pkg/consumer"
	"github.com/dunstall/wombatclient/pkg/producer"
)

const (
  Addr = "localhost:3110"
  Partition = 0
)

func ProducerConnect(t *testing.T) producer.Producer {
  p, err := producer.Connect(Addr)
  if err != nil {
    t.Fatal("failed to create producer connection", err)
  }
  return p
}

func ConsumerConnect(t *testing.T) consumer.Consumer {
  c, err := consumer.Connect(Addr)
  if err != nil {
    t.Fatal("failed to create consumer connection", err)
  }
  return c
}

func Stat(t *testing.T, c consumer.Consumer) uint32 {
  size, ok := c.Stat(Partition)
  if !ok {
    t.Fatal("failed to get partition stat")
  }
  return size
}

func Produce(t *testing.T, p producer.Producer, b []byte) {
  ok := p.Produce(Partition, b)
  if !ok {
    t.Fatalf("failed to produce data")
  }
}

func Consume(t *testing.T, c consumer.Consumer, offset uint32) []byte {
  b, ok := c.Consume(Partition, offset)
  if !ok {
    t.Fatalf("failed to consume data")
  }
  return b
}
