package main

import (
	"encoding/binary"
  "fmt"
  "net"
)

type ProduceRecord struct {
  data []byte
}

func NewProduceRecord(data []byte) ProduceRecord {
  return ProduceRecord{data}
}

func (r *ProduceRecord) Encode() []byte {
	b := make([]byte, 4)
	binary.BigEndian.PutUint32(b, uint32(len(r.data)))
	b = append(b, r.data...)
  return b
}

func main() {
  conn, err := net.Dial("tcp", "localhost:3111")
  if err != nil {
    fmt.Println(err)
    return
  }
  defer conn.Close()

  for {
    r := NewProduceRecord([]byte("testdata"))
    n, err := conn.Write(r.Encode())
    if err != nil {
      fmt.Println(err)
      return
    }
    fmt.Println("write", n)
  }
}
