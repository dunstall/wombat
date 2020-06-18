# Port allocation: 51XX
#   5101: Unreachable

# TODO(AD) Docker compose

import socketserver
import struct
import time

"""
Leader OK

Handle the normal case of replica sending an offset of 0 and respond with
a stream of data.

PORT: 5100
"""

class LeaderOkHandler(socketserver.BaseRequestHandler):
    allow_reuse_address = True

    def handle(self):
        data = self.request.recv(4).strip()
        offset = struct.unpack("!L", data)[0]

        for _ in range(5):
            self.request.sendall(b"\x04\x01\x02\x03\x04")
            time.sleep(0.1)


"""
Leader unreachable.

Port: 5101
"""
# Nothing to run.


"""
Leader accept request and immediately close.

Replica should keep trying to connect.

Port: 5102
"""

class LeaderCloseImmediatelyHandler(socketserver.BaseRequestHandler):
    allow_reuse_address = True

    def handle(self):
        self.request.close()


"""
Leader write single byte and close.

Replica should keep re-connecting and receiving the stream of data (giving
a new offset each time).

Port: 5103
"""

class LeaderWriteAndCloseHandler(socketserver.BaseRequestHandler):
    allow_reuse_address = True

    log = b"\x04\x01\x02\x03\x04" * 5

    def handle(self):
        data = b''
        while len(data) < 4:
            data += self.request.recv(4 - len(data)).strip()
        offset = struct.unpack("!L", data)[0]

        self.request.sendall(self.log[offset:offset+1])

        self.request.close()


def run(handler, port):
    with socketserver.TCPServer(("localhost", port), handler) as server:
        server.serve_forever()
