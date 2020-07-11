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

func TestNewMessageOk(t *testing.T) {
	var kind uint32 = ConsumeResponse
	var partitionID uint32 = 0xffaa
	payload := []byte{1, 2, 3, 4}
	m, ok := NewMessage(kind, partitionID, payload)
	if !ok {
		t.Errorf("failed to create message")
	}
	if kind != m.Kind() {
		t.Errorf("m.Kind() != %#v, actual %#v", kind, m.Kind())
	}
	if partitionID != m.PartitionID() {
		t.Errorf("m.PartitionID() != %#v, actual %#v", partitionID, m.PartitionID())
	}
	if !reflect.DeepEqual(payload, m.Payload()) {
		t.Errorf("m.Payload() != %#v, actual %#v", payload, m.Payload())
	}
}

func TestNewMessageSizeZero(t *testing.T) {
	_, ok := NewMessage(0, 0, []byte{})
	if ok {
		t.Errorf("expected error creating message")
	}
}

func TestNewMessageExceedSizeLimit(t *testing.T) {
	_, ok := NewMessage(0, 0, make([]byte, 513))
	if ok {
		t.Errorf("expected error creating message")
	}
}

func TestEncodeMessage(t *testing.T) {
	payload := []byte{1, 2, 3, 4}
	m, ok := NewMessage(ConsumeResponse, 0xffaa, payload)
	if !ok {
		t.Errorf("failed to create message")
	}
	expected := []byte{
		0x00, 0x00, 0x00, 0x03, // Type
		0x00, 0x00, 0xff, 0xaa, // Partition ID
		0x00, 0x00, 0x00, 0x04, // Payload size
		0x01, 0x02, 0x03, 0x04, // Payload
	}
	if !reflect.DeepEqual(expected, m.Encode()) {
		t.Errorf("m.Encode() != %#v, actual %#v", expected, m.Encode())
	}
}

func TestDecodeMessageOk(t *testing.T) {
	encoded := []byte{
		0x00, 0x00, 0x00, 0x03, // Type
		0x00, 0x00, 0xff, 0xaa, // Partition ID
		0x00, 0x00, 0x00, 0x04, // Payload size
		// Payload with padded bytes (which should be removed as size is only 4)
		0x01, 0x02, 0x03, 0x04, 0x00, 0x00,
	}
	m, ok := DecodeMessage(encoded)
	if !ok {
		t.Errorf("m.Decode(%#v) != true", encoded)
	}

	expected, _ := NewMessage(ConsumeResponse, 0xffaa, []byte{1, 2, 3, 4})
	if !reflect.DeepEqual(expected, m) {
		t.Errorf("m.Decode(%#v) != %#v, actual %#v", encoded, expected, m)
	}
}

func TestDecodeMessagePayloadTooSmall(t *testing.T) {
	encoded := []byte{
		0x00, 0x00, 0x00, 0x03, // Type
		0x00, 0x00, 0xff, 0xaa, // Partition ID
		0x00, 0x00, 0x00, 0x04, // Payload size
		// Payload smaller than size in header.
		0x01, 0x02,
	}
	_, ok := DecodeMessage(encoded)
	if ok {
		t.Errorf("m.Decode(%#v) != false", encoded)
	}
}
