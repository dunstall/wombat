// +build system

package tests

import (
	"fmt"
	"testing"

	"github.com/dunstall/wombatclient/pkg/consumer"
	"github.com/dunstall/wombatclient/pkg/producer"
	"github.com/dunstall/wombatclient/pkg/record"
	"github.com/google/uuid"
)

func TestEndToEnd(t *testing.T) {
	producer, err := producer.New("server.1.wombat:3110")
	if err != nil {
		t.Error(err)
	}
	defer producer.Close()

	topic := uuid.New().String()
	r := record.NewProduceRecord(topic, []byte{}, []byte{5, 6, 7, 8})
	for i := 0; i != 1000; i++ {
		fmt.Println("produce")
		if err = producer.Send(r); err != nil {
			t.Error(err)
		}
	}

	c, err := consumer.New("data/consumer.conf")
	if err != nil {
		t.Fatal(err)
	}
	if err := c.Subscribe(topic); err != nil {
		t.Fatal(err)
	}

	for i := 0; i != 1000; i++ {
		fmt.Println("consumed", i)
		res, chunk, err := c.Poll()
		if err != nil {
			t.Fatal(err)
		}

		if err = c.Commit(res, chunk); err != nil {
			t.Fatal(err)
		}
	}
}
