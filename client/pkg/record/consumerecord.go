package record

import (
	"encoding/binary"
	"io"
)

const (
	consumeRecordType = 3
)

type ConsumeRecord struct {
	next_offset uint64
	key         []byte
	val         []byte
}

func NewConsumeRecord(next_offset uint64, key []byte, val []byte) ConsumeRecord {
	return ConsumeRecord{
		next_offset,
		key,
		val,
	}
}

func (r *ConsumeRecord) NextOffset() uint64 {
	return r.next_offset
}

func (r *ConsumeRecord) Key() []byte {
	return r.key
}

func (r *ConsumeRecord) Val() []byte {
	return r.val
}

func ReadConsumeRecord(reader io.Reader) (ConsumeRecord, error) {
	r := ConsumeRecord{}

	b := make([]byte, 2)
	_, err := reader.Read(b) // TODO(AD) Read until full or err
	if err != nil {
		return r, err
	}

	if binary.BigEndian.Uint16(b) != consumeRecordType {
		// TODO(AD) Unrecognized error.
	}

	b = make([]byte, 8)
	_, err = reader.Read(b) // TODO(AD) Read until full or err
	if err != nil {
		return r, err
	}
	r.next_offset = binary.BigEndian.Uint64(b)

	_, err = reader.Read(b) // TODO(AD) Read until full or err
	if err != nil {
		return r, err
	}
	r.key = make([]byte, binary.BigEndian.Uint64(b))
	_, err = reader.Read(r.key)
	if err != nil {
		return r, err
	}

	_, err = reader.Read(b) // TODO(AD) Read until full or err
	if err != nil {
		return r, err
	}
	r.val = make([]byte, binary.BigEndian.Uint64(b))
	_, err = reader.Read(r.val)
	if err != nil && err != io.EOF {
		return r, err
	}

	return r, nil
}
