package record

import (
	"reflect"
	"testing"
)

func TestNewMessageHeaderOk(t *testing.T) {
	var kind uint32 = ConsumeResponse
	var partitionID uint32 = 0xffaa
	var payloadSize uint32 = 0xff
	m, ok := NewMessageHeader(kind, partitionID, payloadSize)
	if !ok {
		t.Errorf("failed to create message header")
	}
	if kind != m.Kind() {
		t.Errorf("m.Kind() != %#v, actual %#v", kind, m.Kind())
	}
	if partitionID != m.PartitionID() {
		t.Errorf("m.PartitionID() != %#v, actual %#v", partitionID, m.PartitionID())
	}
	if payloadSize != m.PayloadSize() {
		t.Errorf("m.PayloadSize() != %#v, actual %#v", payloadSize, m.PayloadSize())
	}
}

func TestNewMessageHeaderSizeZero(t *testing.T) {
	_, ok := NewMessageHeader(0, 0, 0)
	if ok {
		t.Errorf("expected error creating message")
	}
}

func TestNewMessageHeaderExceedSizeLimit(t *testing.T) {
	_, ok := NewMessageHeader(0, 0, 513)
	if ok {
		t.Errorf("expected error creating message")
	}
}

func TestEncodeMessageHeader(t *testing.T) {
	m, ok := NewMessageHeader(ConsumeResponse, 0xffaa, 0xff)
	if !ok {
		t.Errorf("failed to create message")
	}

	expected := []byte{
		0x00, 0x00, 0x00, 0x03, // Type
		0x00, 0x00, 0xff, 0xaa, // Partition ID
		0x00, 0x00, 0x00, 0xff, // Payload size
	}
	if !reflect.DeepEqual(expected, m.Encode()) {
		t.Errorf("m.Encode() != %#v, actual %#v", expected, m.Encode())
	}
}

func TestDecodeMessageHeaderOk(t *testing.T) {
	encoded := []byte{
		0x00, 0x00, 0x00, 0x03, // Type
		0x00, 0x00, 0xff, 0xaa, // Partition ID
		0x00, 0x00, 0x00, 0xff, // Payload size
	}
	m, ok := DecodeMessageHeader(encoded)
	if !ok {
		t.Errorf("m.Decode(%#v) != true", encoded)
	}

	expected, _ := NewMessageHeader(ConsumeResponse, 0xffaa, 0xff)
	if !reflect.DeepEqual(expected, m) {
		t.Errorf("m.Decode(%#v) != %#v, actual %#v", encoded, expected, m)
	}
}

func TestDecodeMessageHeaderTooSmall(t *testing.T) {
	encoded := []byte{
		0x00, 0x00, 0x00, 0x03, // Type
		0x00, 0x00, 0xff, 0xaa, // Partition ID
		0x00, 0x00, // Payload size too small
	}
	_, ok := DecodeMessageHeader(encoded)
	if ok {
		t.Errorf("m.Decode(%#v) != false", encoded)
	}
}

func TestDecodeMessageHeaderExceedsLimit(t *testing.T) {
	encoded := []byte{
		0x00, 0x00, 0x00, 0x03, // Type
		0x00, 0x00, 0xff, 0xaa, // Partition ID
		0x00, 0x0a, 0xff, 0xff, // Payload too large.
	}
	_, ok := DecodeMessageHeader(encoded)
	if ok {
		t.Errorf("m.Decode(%#v) != false", encoded)
	}
}
