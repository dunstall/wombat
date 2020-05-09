package record

import (
	"reflect"
	"testing"
)

func TestEncode(t *testing.T) {
	r := NewProduceRecord("mytopic", []byte{1, 2, 3, 4}, []byte{5, 6, 7, 8})
	r.SetPartition(0xffaa)

	buf := r.Encode()
	expected := []byte{
		0, 0, 0, 0, 0, 0, 0, 7, 109, 121, 116, 111, 112, 105, 99, 0, 0, 0xff,
		0xaa, 0, 0, 0, 0, 0, 0, 0, 4, 1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 4, 5,
		6, 7, 8,
	}
	if !reflect.DeepEqual(buf, expected) {
		t.Errorf("r.Encode() = %v, expected %v", buf, expected)
	}
}
