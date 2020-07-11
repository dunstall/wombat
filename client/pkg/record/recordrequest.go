package record

import (
	"encoding/binary"
)

type RecordRequest struct {
	offset uint32
}

func NewRecordRequest(offset uint32) RecordRequest {
	return RecordRequest{offset}
}

func (rr *RecordRequest) Offset() uint32 {
	return rr.offset
}

func (rr *RecordRequest) Encode() []byte {
	b := make([]byte, uint32Size)
	binary.BigEndian.PutUint32(b, rr.offset)
	return b
}

func DecodeRecordRequest(encoded []byte) (RecordRequest, bool) {
	if len(encoded) < uint32Size {
		return RecordRequest{}, false
	}
	return NewRecordRequest(binary.BigEndian.Uint32(encoded[:uint32Size])), true
}
