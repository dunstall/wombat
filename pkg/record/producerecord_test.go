package record

import (
	"reflect"
	"testing"
)

func TestEncodeProduceRecord(t *testing.T) {
	r := NewProduceRecord("mytopic", []byte{1, 2, 3, 4}, []byte{5, 6, 7, 8})
	r.SetPartition(0xffaa)

	buf := r.Encode()
	expected := []byte{
		0, 0, // Type

		0, 0, 0, 0, 0, 0, 0, 7, // Topic size
		109, 121, 116, 111, 112, 105, 99, // Topic

		0, 0, 0xff, 0xaa, // Partition

		0, 0, 0, 0, 0, 0, 0, 4, // Key size
		1, 2, 3, 4, // Key

		0, 0, 0, 0, 0, 0, 0, 4, // Value size
		5, 6, 7, 8, // Value
	}
	if !reflect.DeepEqual(buf, expected) {
		t.Errorf("r.Encode() = %v, expected %v", buf, expected)
	}
}
