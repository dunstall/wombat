package consumer

import (
	"fmt"
	"testing"
	"time"
)

func TestX(t *testing.T) {
	o, err := newOffsets([]string{"192.168.48.6:2181"}, time.Second*10)
	if err != nil {
		t.Fatal(err)
	}

	if err = o.commit(Partition{"mytopic", 0xff}, 0xafa3410); err != nil {
		t.Fatal(err)
	}

	p, err := o.lookup(Partition{"mytopic", 0xff})
	if err != nil {
		t.Fatal(err)
	}

	fmt.Println(p)

	if p != 0xafa3410 {
		t.Fatal("bad")
	}
}
