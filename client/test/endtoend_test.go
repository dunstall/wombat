// +build system

package tests

import (
	"io"
	"testing"
	"time"

	"github.com/dunstall/wombatclient/pkg/consumer"
	"github.com/dunstall/wombatclient/pkg/consumer/membership"
	"github.com/dunstall/wombatclient/pkg/producer"
	"github.com/dunstall/wombatclient/pkg/record"
	"github.com/google/uuid"
)

func TestSinglePartition(t *testing.T) {
	producer, err := producer.New("localhost:3110")
	if err != nil {
		t.Error(err)
	}

	topic := uuid.New().String()
	r := record.NewProduceRecord(topic, []byte{1, 2, 3, 4}, []byte{5, 6, 7, 8})
	for i := 0; i != 1000; i++ {
		if err = producer.Send(r); err != nil {
			t.Error(err)
		}
	}

	c, err := consumer.New("data/consumer.conf")
	if err != nil {
		t.Fatal(err)
	}
	c.Subscribe(topic)

	for {
		partition := membership.Chunk{topic, 1}
		res, err := c.Poll()
		if err == io.EOF {
			continue
		}
		if err != nil {
			t.Fatal(err)
		}

		if err = c.Commit(res, partition); err != nil {
			t.Fatal(err)
		}
	}

	time.Sleep(time.Second * 100)
}
