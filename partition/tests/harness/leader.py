# Port allocation: 51XX
#   5101: Unreachable

# TODO(AD) Docker compose

import socketserver
import struct
import time

class LeaderOkHandler(socketserver.BaseRequestHandler):
    allow_reuse_address = True

    def handle(self):
        data = self.request.recv(4).strip()
        offset = struct.unpack("!L", data)[0]
        print("offset {} from {}".format(offset, self.client_address[0]))

        for _ in range(10):
            self.request.sendall(b"\x04\x01\x01\x01\x01")
            time.sleep(1)


def leader_ok():
    HOST, PORT = "localhost", 5100
    with socketserver.TCPServer((HOST, PORT), LeaderOkHandler) as server:
        server.serve_forever()


class LeaderCloseImmediatelyHandler(socketserver.BaseRequestHandler):
    def handle(self):
        self.request.close()


def leader_close_immediately():
    HOST, PORT = "localhost", 5102
    with socketserver.TCPServer((HOST, PORT), LeaderCloseImmediatelyHandler) as server:
        server.serve_forever()

class LeaderWriteAndCloseHandler(socketserver.BaseRequestHandler):
    allow_reuse_address = True

    log = b"\x04\x01\x01\x01\x01" * 10

    def handle(self):
        data = self.request.recv(4).strip()
        offset = struct.unpack("!L", data)[0]
        print("offset {} from {}".format(offset, self.client_address[0]))

        self.request.sendall(self.log[offset:offset+1])

def leader_write_and_close():
    HOST, PORT = "localhost", 5103
    with socketserver.TCPServer((HOST, PORT), LeaderWriteAndCloseHandler) as server:
        server.serve_forever()
