package record

import (
	"reflect"
	"testing"
)

func TestNewRecordRequest(t *testing.T) {
	var offset uint32 = 0xff
	rr := NewRecordRequest(offset)
	if offset != rr.Offset() {
		t.Errorf("r.Offset() != %#v, actual %#v", offset, rr.Offset())
	}
}

func TestEncodeRecordRequest(t *testing.T) {
	var offset uint32 = 0xff
	rr := NewRecordRequest(offset)

	expected := []byte{0, 0, 0, 0xff}
	if !reflect.DeepEqual(expected, rr.Encode()) {
		t.Errorf("rr.Encode() != %#v, actual %#v", expected, rr.Encode())
	}
}

func TestDecodeRecordRequestOk(t *testing.T) {
	// Encoded record with appended padding that should be discarded.
	encoded := []byte{0, 0, 0, 0xff}
	rr, ok := DecodeRecordRequest(encoded)
	if !ok {
		t.Errorf("rr.Decode(%#v) != true", encoded)
	}

	expected := NewRecordRequest(0xff)
	if !reflect.DeepEqual(expected, rr) {
		t.Errorf("rr.Decode(%#v) != %#v, actual %#v", encoded, expected, rr)
	}
}

func TestDecodeRequestRecordMissingOffset(t *testing.T) {
	// Requires 4 bytes for offset.
	encoded := []byte{0xff, 0xff}
	_, ok := DecodeRecord(encoded)
	if ok {
		t.Errorf("rr.Decode(%#v) != false", encoded)
	}
}
