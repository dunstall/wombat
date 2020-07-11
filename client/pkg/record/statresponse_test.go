package record

import (
	"reflect"
	"testing"
)

func TestNewStatResponse(t *testing.T) {
	var size uint32 = 0xff
	sr := NewStatResponse(size)
	if size != sr.Size() {
		t.Errorf("r.Size() != %#v, actual %#v", size, sr.Size())
	}
}

func TestEncodeStatResponse(t *testing.T) {
	var size uint32 = 0xff
	sr := NewStatResponse(size)

	expected := []byte{0, 0, 0, 0xff}
	if !reflect.DeepEqual(expected, sr.Encode()) {
		t.Errorf("sr.Encode() != %#v, actual %#v", expected, sr.Encode())
	}
}

func TestDecodeStatResponseOk(t *testing.T) {
	// Encoded record with appended padding that should be discarded.
	encoded := []byte{0, 0, 0, 0xff}
	sr, ok := DecodeStatResponse(encoded)
	if !ok {
		t.Errorf("sr.Decode(%#v) != true", encoded)
	}

	expected := NewStatResponse(0xff)
	if !reflect.DeepEqual(expected, sr) {
		t.Errorf("sr.Decode(%#v) != %#v, actual %#v", encoded, expected, sr)
	}
}

func TestDecodeRequestRecordMissingSize(t *testing.T) {
	// Requires 4 bytes for size.
	encoded := []byte{0xff, 0xff}
	_, ok := DecodeRecord(encoded)
	if ok {
		t.Errorf("sr.Decode(%#v) != false", encoded)
	}
}
