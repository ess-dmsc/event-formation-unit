import string
import socket
import sys
import time
import threading


class SimpleSocket:
  def __init__(self, hostname = "localhost", port = 8888, timeout = 2):
    self.access_semaphor = threading.Semaphore(1)
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

  def SendCommand(self, cmd):
    self.access_semaphor.acquire()
    self.sock.send(cmd + '\n')
    self.access_semaphor.release()

  def Ask(self, cmd):
    self.access_semaphor.acquire()
    self.sock.send(cmd + '\n')
    reply = self.sock.recv(2048).strip('\n')
    self.access_semaphor.release()
    return reply
