package main

import (
	"fmt"

	"github.com/dunstall/wombatclient/pkg/consumer"
	"github.com/dunstall/wombatclient/pkg/producer"
)

func main() {
	p, err := producer.Connect("localhost:3110")
	if err != nil {
		fmt.Println(err)
		return
	}
	defer p.Close()

	c, err := consumer.Connect("localhost:3110")
	if err != nil {
		fmt.Println(err)
		return
	}
	defer c.Close()

	for {
		if !p.Produce(0, []byte("test")) {
			fmt.Println("produce failed")
		}

		// TODO(AD) Incr offset - start at stat.offset
		b, ok := c.Consume(0, 0)
		if !ok {
			fmt.Println("consume failed")
		}
		fmt.Println("consumed data", string(b))

		fmt.Println(c.Stat(0))
	}
}
