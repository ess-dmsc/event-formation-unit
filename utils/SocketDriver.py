import string
import socket
import sys
import time

class SimpleSocket:

    def __init__(self, hostname, port = 8888, timeout = 2):
        self.hostname = hostname
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        except socket.error, msg:
            sys.stderr.write("socket() [Socket connection error] Cannot connect to %s, error: %s\n" % (hostname, msg[0]))
            sys.exit(1)

        self.sock.settimeout(timeout)

        try:
            self.sock.connect((hostname, port))
        except socket.error, msg:
            sys.stderr.write("connect() [Socket connection error] Cannot connect to %s:%d, error: %s\n" % (hostname, port, msg[0]))
            sys.exit(2)


    def __del__(self):
        self.sock.close()

    def SendCommand(self, cmd):
        self.sock.send(cmd + '\n')

    def Ask(self, cmd):
        self.sock.send(cmd + '\n')
        return self.sock.recv(2048)

    def set_keepalives(self):
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 2)
