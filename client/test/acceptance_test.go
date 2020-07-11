package test

import (
  "math/rand"
	"reflect"
	"testing"
)

func TestStatIsUpdated(t *testing.T) {
  p := ProducerConnect(t)
  defer p.Close()
  c := ConsumerConnect(t)
  defer c.Close()

  initial_size := Stat(t, c, 0)

  b := []byte("mytestdata")
  Produce(t, p, b, 0)

  updated_size := Stat(t, c, 0)

  expected_size := initial_size + uint32(len(b)) + 4
  if expected_size != updated_size {
    t.Errorf("updated_size != %d, actual %d", expected_size, updated_size)
  }
}

func TestConsumeProducedRecord(t *testing.T) {
  p := ProducerConnect(t)
  defer p.Close()
  c := ConsumerConnect(t)
  defer c.Close()

  offset := Stat(t, c, 0)

  b := make([]byte, 10)
  rand.Read(b)
  Produce(t, p, b, 0)

  received := Consume(t, c, offset, 0)
  if !reflect.DeepEqual(b, received) {
    t.Errorf("c.Consume(%d) != %#v, actual %#v", offset, b, received)
  }
}

func TestProduceDifferentPartitions(t *testing.T) {
  p := ProducerConnect(t)
  defer p.Close()
  c := ConsumerConnect(t)
  defer c.Close()

  offset1 := Stat(t, c, 0)
  offset2 := Stat(t, c, 1)

  b1 := make([]byte, 10)
  rand.Read(b1)
  Produce(t, p, b1, 0)

  b2 := make([]byte, 20)
  rand.Read(b2)
  Produce(t, p, b2, 1)

  received1 := Consume(t, c, offset1, 0)
  if !reflect.DeepEqual(b1, received1) {
    t.Errorf("c.Consume(%d) != %#v, actual %#v", offset1, b1, received1)
  }

  received2 := Consume(t, c, offset2, 1)
  if !reflect.DeepEqual(b2, received2) {
    t.Errorf("c.Consume(%d) != %#v, actual %#v", offset1, b2, received2)
  }
}
