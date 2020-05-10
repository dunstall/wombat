package record

import (
	"encoding/binary"
	"fmt"
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
  // TODO not blocking
	t, err := reader.Read(b)  // TODO check len and err
	if err != nil {
    fmt.Println("error of read")
		return r, err
	}

  fmt.Println(t)

	_ = binary.BigEndian.Uint16(b) // TODO
	// TODO check type

	b = make([]byte, 8)
	_, err = reader.Read(b)  // TODO check len and err
	if err != nil {
		return r, err
	}
	r.next_offset = binary.BigEndian.Uint64(b)
  fmt.Println(r.next_offset)

	_, err = reader.Read(b)  // TODO check len and err
	if err != nil {
		return r, err
	}
	r.key = make([]byte, binary.BigEndian.Uint64(b))
  fmt.Println(len(r.key))
  n, err := reader.Read(r.key)
	if n != len(r.key) { // TODO read until filled
		return r, fmt.Errorf("failed to read consume record")
	}
	if err != nil {
		return r, err
	}

	_, err = reader.Read(b)  // TODO check len and err
	if err != nil {
		return r, err
	}
	r.val = make([]byte, binary.BigEndian.Uint64(b))
  fmt.Println(len(r.val))
	n, err = reader.Read(r.val)
	if n != len(r.val) { // TODO read until filled
		return r, fmt.Errorf("failed to read consume record")
	}
	if err != nil && err != io.EOF {
		return r, err
	}

	return r, nil
}
