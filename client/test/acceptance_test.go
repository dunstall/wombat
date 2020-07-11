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

  initial_size := Stat(t, c)

  b := []byte("mytestdata")
  Produce(t, p, b)

  updated_size := Stat(t, c)

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

  offset := Stat(t, c)

  b := make([]byte, 10)
  rand.Read(b)
  Produce(t, p, b)

  received := Consume(t, c, offset)
  if !reflect.DeepEqual(b, received) {
    t.Errorf("c.Consume(%d) != %#v, actual %#v", offset, b, received)
  }
}
