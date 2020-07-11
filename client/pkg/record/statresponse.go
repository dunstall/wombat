package record

import (
	"encoding/binary"
)

type StatResponse struct {
	size uint32
}

func NewStatResponse(size uint32) StatResponse {
	return StatResponse{size}
}

func (sr *StatResponse) Size() uint32 {
	return sr.size
}

func (sr *StatResponse) Encode() []byte {
	b := make([]byte, uint32Size)
	binary.BigEndian.PutUint32(b, sr.size)
	return b
}

func DecodeStatResponse(encoded []byte) (StatResponse, bool) {
	if len(encoded) < uint32Size {
		return StatResponse{}, false
	}
	return NewStatResponse(binary.BigEndian.Uint32(encoded[:uint32Size])), true
}
