package main

import (
	"encoding/binary"
	"fmt"
	"net"
)

const (
	ProduceRequest = iota
	ConsumeRequest
	kReplicaRequest
	kConsumeResponse
	kReplicaResponse
)

type MessageHeader struct {
	Kind        uint32
	PartitionID uint32
	PayloadSize uint32
}

func (h *MessageHeader) Encode() []byte {
	b := make([]byte, 12)
	binary.BigEndian.PutUint32(b[:4], h.Kind)
	binary.BigEndian.PutUint32(b[4:8], h.PartitionID)
	binary.BigEndian.PutUint32(b[8:12], h.PayloadSize)
	return b
}

type Message struct {
	Header  MessageHeader
	Payload []byte
}

func (m *Message) Encode() []byte {
	b := m.Header.Encode()
	b = append(b, m.Payload...)
	return b
}

type Record struct {
	data []byte
}

func NewRecord(data []byte) Record {
	return Record{data}
}

func (r *Record) Encode() []byte {
	b := make([]byte, 4)
	binary.BigEndian.PutUint32(b, uint32(len(r.data)))
	b = append(b, r.data...)
	return b
}

func main() {
	conn, err := net.Dial("tcp", "localhost:3110")
	if err != nil {
		fmt.Println(err)
		return
	}
	defer conn.Close()

	for {
		r := NewRecord([]byte("testdata"))

		h := MessageHeader{
			Kind:        ProduceRequest,
			PartitionID: 0,
			PayloadSize: uint32(len(r.Encode())),
		}
		m := Message{
			Header:  h,
			Payload: r.Encode(),
		}

		n, err := conn.Write(m.Encode())
		if err != nil {
			fmt.Println(err)
			return
		}
		fmt.Println("write", n)
	}
}
