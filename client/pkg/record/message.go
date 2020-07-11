package record

import (
	"encoding/binary"
)

const (
	ProduceRequest = iota
	ConsumeRequest
	ReplicaRequest
	ConsumeResponse
	ReplicaResponse
)

const (
	MessageHeaderSize = 12
	PayloadLimit      = 512
)

type MessageHeader struct {
	kind        uint32
	partitionID uint32
	payloadSize uint32
}

func NewMessageHeader(kind uint32, partitionID uint32, payloadSize uint32) (MessageHeader, bool) {
	if payloadSize > PayloadLimit {
		return MessageHeader{}, false
	}
	return MessageHeader{
		kind:        kind,
		partitionID: partitionID,
		payloadSize: payloadSize,
	}, true
}

func (h *MessageHeader) Kind() uint32 {
	return h.kind
}

func (h *MessageHeader) PartitionID() uint32 {
	return h.partitionID
}

func (h *MessageHeader) PayloadSize() uint32 {
	return h.payloadSize
}

func (h *MessageHeader) Encode() []byte {
	b := make([]byte, MessageHeaderSize)
	binary.BigEndian.PutUint32(b[:4], h.kind)
	binary.BigEndian.PutUint32(b[4:8], h.partitionID)
	binary.BigEndian.PutUint32(b[8:12], h.payloadSize)
	return b
}

func DecodeMessageHeader(b []byte) (MessageHeader, bool) {
	if len(b) < MessageHeaderSize {
		return MessageHeader{}, false
	}
	return NewMessageHeader(
		binary.BigEndian.Uint32(b[0:4]),
		binary.BigEndian.Uint32(b[4:8]),
		binary.BigEndian.Uint32(b[8:12]),
	)
}

type Message struct {
	header  MessageHeader
	payload []byte
}

func NewMessage(kind uint32, partitionID uint32, payload []byte) (Message, bool) {
	header, ok := NewMessageHeader(kind, partitionID, uint32(len(payload)))
	if !ok {
		return Message{}, false
	}
	return Message{header, payload}, true
}

func (m *Message) Kind() uint32 {
	return m.header.kind
}

func (m *Message) PartitionID() uint32 {
	return m.header.partitionID
}

func (m *Message) Payload() []byte {
	return m.payload
}

func (m *Message) Encode() []byte {
	b := m.header.Encode()
	return append(b, m.payload...)
}

func DecodeMessage(b []byte) (Message, bool) {
	header, ok := DecodeMessageHeader(b)
	if !ok {
		return Message{}, false
	}

	if uint32(len(b)) < MessageHeaderSize+header.PayloadSize() {
		return Message{}, false
	}
	return Message{
		header,
		b[MessageHeaderSize : MessageHeaderSize+header.PayloadSize()],
	}, true
}
