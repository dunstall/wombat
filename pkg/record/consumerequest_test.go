package record

import (
	"reflect"
	"testing"
)

func TestEncodeConsumeRequest(t *testing.T) {
	r := NewConsumeRequest("mytopic", 0xffaa, 0xaabbccddaabbccdd)

	buf := r.Encode()
	expected := []byte{
		0, 1, // Type
		0, 0, 0, 0, 0, 0, 0, 7, // Topic size
		109, 121, 116, 111, 112, 105, 99, // Topic
		0xaa, 0xbb, 0xcc, 0xdd, 0xaa, 0xbb, 0xcc, 0xdd, // Offset
		0, 0, 0xff, 0xaa, // Partition
	}
	if !reflect.DeepEqual(buf, expected) {
		t.Errorf("r.Encode() = %v, expected %v", buf, expected)
	}
}
