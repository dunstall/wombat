package producer

import (
  "github.com/dunstall/wombatclient/pkg/record"
)

type Producer struct {}

func (p *Producer) New(server string) Producer {
	return Producer{}
}

func (p *Producer) send(record record.ProduceRecord) error {
	return nil
}
