package record

import (
	"encoding/binary"
)

type ProduceRecord struct {
	topic     string // TODO byte?
	partition uint32
	key       []byte
	val       []byte
}

func NewProduceRecord(topic string, key []byte, val []byte) ProduceRecord {
	return ProduceRecord{
		topic,
		0,
		key,
		val,
	}
}

func (r *ProduceRecord) Topic() string {
	return r.topic
}

func (r *ProduceRecord) Partition() uint32 {
	return r.partition
}

func (r *ProduceRecord) Key() []byte {
	return r.key
}

func (r *ProduceRecord) Val() []byte {
	return r.val
}

func (r *ProduceRecord) SetPartition(partition uint32) {
	r.partition = partition
}

func (r *ProduceRecord) Encode() []byte {
	// Topic
	b := make([]byte, 8)
	binary.BigEndian.PutUint64(b, uint64(len(r.topic)))
	b = append(b, []byte(r.topic)...)

	// Partition
	partition_buf := make([]byte, 4)
	binary.BigEndian.PutUint32(partition_buf, r.partition)
	b = append(b, partition_buf...)

	// Key
	key_buf := make([]byte, 8)
	binary.BigEndian.PutUint64(key_buf, uint64(len(r.key)))
	b = append(b, key_buf...)
	b = append(b, r.key...)

	// Value
	val_buf := make([]byte, 8)
	binary.BigEndian.PutUint64(val_buf, uint64(len(r.val)))
	b = append(b, val_buf...)
	b = append(b, r.val...)

	return b
}
