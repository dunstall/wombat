// +build system

package tests

import (
	"fmt"
	"testing"

	"github.com/dunstall/wombatclient/pkg/consumer"
	"github.com/dunstall/wombatclient/pkg/consumer/membership"
	"github.com/dunstall/wombatclient/pkg/producer"
	"github.com/dunstall/wombatclient/pkg/record"
	"github.com/google/uuid"
)

func TestEndToEnd(t *testing.T) {
	producer, err := producer.New("localhost:3110")
	if err != nil {
		t.Error(err)
	}
	defer producer.Close()

	topic := uuid.New().String()
	r := record.NewProduceRecord(topic, []byte{}, []byte{5, 6, 7, 8})
	for i := 0; i != 10000; i++ {
		if err = producer.Send(r); err != nil {
			t.Error(err)
		}
	}

	fmt.Println("Successfully written records")

	c, err := consumer.New("data/consumer.conf")
	if err != nil {
		t.Fatal(err)
	}
	if err := c.Subscribe(topic); err != nil {
		t.Fatal(err)
	}

	for {
		res, err := c.Poll()
		if err != nil {
			t.Fatal(err)
		}

		// TODO wrongs types
		partition := membership.Chunk{topic, uint32(res.NextOffset())}
		if err = c.Commit(res, partition); err != nil {
			t.Fatal(err)
		}
	}
}
