package record

import (
	"encoding/binary"
)

const (
	consumeRequestType = 1
)

type ConsumeRequest struct {
	topic     string
	partition uint32
	offset    uint64
}

func NewConsumeRequest(topic string, partition uint32, offset uint64) ConsumeRequest {
	return ConsumeRequest{
		topic,
		partition,
		offset,
	}
}

func (r *ConsumeRequest) Topic() string {
	return r.topic
}

func (r *ConsumeRequest) Partition() uint32 {
	return r.partition
}

func (r *ConsumeRequest) Offset() uint64 {
	return r.offset
}

func (r *ConsumeRequest) Encode() []byte {
	b := make([]byte, 2)
	binary.BigEndian.PutUint16(b, uint16(consumeRequestType))

	// Topic
	topic_buf := make([]byte, 8)
	binary.BigEndian.PutUint64(topic_buf, uint64(len(r.topic)))
	b = append(b, topic_buf...)
	b = append(b, []byte(r.topic)...)

	// Partition
	partition_buf := make([]byte, 4)
	binary.BigEndian.PutUint32(partition_buf, r.partition)
	b = append(b, partition_buf...)

	// Partition
	offset_buf := make([]byte, 8)
	binary.BigEndian.PutUint64(offset_buf, r.offset)
	b = append(b, offset_buf...)

	return b
}
