package record

import (
	"reflect"
	"testing"
)

func TestNewRecordOk(t *testing.T) {
	b := []byte{1, 2, 3}
	r, ok := NewRecord(b)
	if !ok {
		t.Errorf("failed to create record")
	}
	if !reflect.DeepEqual(r.data, b) {
		t.Errorf("r.Data() != %#v, actual %#v", b, r.Data())
	}
}

func TestNewRecordExceedSizeLimit(t *testing.T) {
	b := make([]byte, 513)
	_, ok := NewRecord(b)
	if ok {
		t.Errorf("expected error creating record")
	}
}

func TestEncode(t *testing.T) {
	b := []byte{1, 2, 3}
	r, ok := NewRecord(b)
	if !ok {
		t.Errorf("failed to create record")
	}

	expected := []byte{0, 0, 0, 3, 1, 2, 3}
	if !reflect.DeepEqual(expected, r.Encode()) {
		t.Errorf("r.Encode() != %#v, actual %#v", expected, r.Encode())
	}
}

func TestDecodeOk(t *testing.T) {
	// Encoded record with appended padding that should be discarded.
	encoded := []byte{0, 0, 0, 3, 1, 2, 3, 0, 0, 0}
	r, ok := DecodeRecord(encoded)
	if !ok {
		t.Errorf("r.Decode(%#v) != true", encoded)
	}

	expected, _ := NewRecord([]byte{1, 2, 3})
	if !reflect.DeepEqual(expected, r) {
		t.Errorf("r.Decode(%#v) != %#v, actual %#v", encoded, expected, r)
	}
}

func TestDecodeMissingSize(t *testing.T) {
	// Requires 4 bytes for size.
	encoded := []byte{0xff, 0xff}
	_, ok := DecodeRecord(encoded)
	if ok {
		t.Errorf("r.Decode(%#v) != false", encoded)
	}
}

func TestDecodePayloadTooSmall(t *testing.T) {
	// Payload must be at least 0xff bytes (but is only 3).
	encoded := []byte{0, 0, 0, 0xff, 1, 2, 3}
	_, ok := DecodeRecord(encoded)
	if ok {
		t.Errorf("r.Decode(%#v) != false", encoded)
	}
}

func TestDecodeSizeExceedsLimit(t *testing.T) {
	encoded := make([]byte, 0xffff)
	encoded[0] = 0
	encoded[1] = 0
	encoded[2] = 0xff
	encoded[3] = 0xff - 0x4
	_, ok := DecodeRecord(encoded)
	if ok {
		t.Errorf("r.Decode(%#v) != false", encoded)
	}
}
