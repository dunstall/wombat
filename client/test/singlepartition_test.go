package tests

import (
	"testing"
	"time"

  "github.com/dunstall/wombatclient/pkg/consumer"
	"github.com/dunstall/wombatclient/pkg/producer"
	"github.com/dunstall/wombatclient/pkg/record"
)

func TestSinglePartition(t *testing.T) {
	producer, err := producer.New("localhost:3110")
	if err != nil {
		t.Error(err)
	}

	r := record.NewProduceRecord("mytopic", []byte{1, 2, 3, 4}, []byte{5, 6, 7, 8})
	r.SetPartition(1)

  for i := 0; i != 100; i++ {
		if err = producer.Send(r); err != nil {
			t.Error(err)
		}
	}

  c, err := consumer.New(
    "mygroup", "localhost:3110", []string{"192.168.48.6:2181"}, time.Second*5,
  )
  if err != nil {
    t.Error(err)
  }

  for i := 0; i != 100; i++ {
    partition := consumer.Partition{"mytopic", 1}
    res, err := c.Poll(partition)
    if err != nil {
      t.Error(err)
    }

    if err = c.Commit(res, partition); err != nil {
      t.Error(err)
    }
  }

  time.Sleep(time.Second * 100)
}
