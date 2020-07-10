package record

import (
	"encoding/binary"
)

const (
	recordSizeLimit = 512
	uint32Size      = 4
)

type Record struct {
	data []byte
}

func NewRecord(data []byte) (Record, bool) {
	if len(data) > recordSizeLimit {
		return Record{}, false
	}
	return Record{data}, true
}

func (r *Record) Data() []byte {
	return r.data
}

func (r *Record) Encode() []byte {
	// Payload size.
	b := make([]byte, uint32Size)
	binary.BigEndian.PutUint32(b, uint32(len(r.data)))
	// Payload.
	b = append(b, r.data...)
	return b
}

func DecodeRecord(encoded []byte) (Record, bool) {
	if len(encoded) < uint32Size {
		return Record{}, false
	}
	size := binary.BigEndian.Uint32(encoded[:uint32Size])

	if uint32(len(encoded)) < uint32Size+size {
		return Record{}, false
	}
	return NewRecord(encoded[uint32Size : uint32Size+size])
}
