/** Copyright (C) 2016 European Spallation Source */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <libs/include/Socket.h>
#include <prototype2/common/Trace.h>

Socket::Socket(Socket::type stype) {
  auto type = (stype == Socket::type::UDP) ? SOCK_DGRAM : SOCK_STREAM;
  auto proto = (stype == Socket::type::UDP) ? IPPROTO_UDP : IPPROTO_TCP;

  if ((s_ = socket(AF_INET, type, proto)) == -1) {
    XTRACE(INIT, ALW, "socket() failed\n");
    exit(1);
  }
}

int Socket::setbuffers(int sndbuf, int rcvbuf) {
  int res = 0;
  if (sndbuf)
    res += setopt(SO_SNDBUF, &sndbuf, sizeof(sndbuf));
  if (rcvbuf)
    res += setopt(SO_RCVBUF, &rcvbuf, sizeof(rcvbuf));
  return res;
}

void Socket::printbuffers(void) {
  XTRACE(IPC, ALW, "Socket receive buffer size: %d\n", getopt(SO_RCVBUF));
  XTRACE(IPC, ALW, "Socket send buffer size: %d\n", getopt(SO_SNDBUF));
}

int Socket::settimeout(int seconds, int usecs) {
  struct timeval timeout;
  timeout.tv_sec = seconds;
  timeout.tv_usec = usecs;
  return setopt(SO_RCVTIMEO, &timeout, sizeof(timeout));
}

void Socket::local(const char *ipaddr, int port) {
  // zero out the structures
  std::memset((char *)&local_, 0, sizeof(local_));
  local_.sin_family = AF_INET;
  local_.sin_port = htons(port);
  inet_aton(ipaddr, &local_.sin_addr);

  // bind socket to port
  int ret = bind(s_, (struct sockaddr *)&local_, sizeof(local_));
  if (ret != 0) {
    std::cout << "bind failed - is port " << port << " already in use?"
              << std::endl;
  }
  assert(ret == 0);
}

void Socket::remote(const char *ipaddr, int port) {
  // zero out the structures
  std::memset((char *)&remote_, 0, sizeof(remote_));
  remote_.sin_family = AF_INET;
  remote_.sin_port = htons(port);
  int ret = inet_aton(ipaddr, &remote_.sin_addr);
  if (ret == 0) {
    std::cout << "invalid ip address " << ipaddr << std::endl;
  }
  assert(ret != 0);
}

int Socket::send(void *buffer, int len) {
  int ret =
      sendto(s_, buffer, len, 0, (struct sockaddr *)&remote_, sizeof(remote_));
  if (ret < 0) {
    std::cout << "unable to send on socket" << std::endl;
    perror("send");
    exit(1); /**< @todo a bit harsh maybe ? */
  }

  return ret;
}

/** */
int Socket::receive(void *buffer, int buflen) {
  socklen_t slen = 0;
  // try to receive some data, this is a blocking call
  return recvfrom(s_, buffer, buflen, 0, (struct sockaddr *)&remote_, &slen);
}

//
// Private methods
//

int Socket::getopt(int option) {
  int optval, ret;
  socklen_t optlen;
  optlen = sizeof(optval);
  if ((ret = getsockopt(s_, SOL_SOCKET, option, (void *)&optval, &optlen)) <
      0) {
    std::cout << "getsockopt() failed" << std::endl;
    return ret;
  }
  return optval;
}

int Socket::setopt(int option, void *value, int size) {
  int ret;
  if ((ret = setsockopt(s_, SOL_SOCKET, option, value, size)) < 0) {
    std::cout << "setsockopt() failed" << std::endl;
  }
  return ret;
}

TCPClient::TCPClient(const char *ipaddr, int port) {
  s_ = socket(AF_INET, SOCK_STREAM, 0);
  if (s_ < 0) {
    std::cout << "TCPSocket(): socket() failed" << std::endl;
  }
  std::memset((char *)&remote_, 0, sizeof(remote_));
  remote_.sin_family = AF_INET;
  remote_.sin_port = htons(port);
  int ret = inet_aton(ipaddr, &remote_.sin_addr);
  if (ret == 0) {
    std::cout << "invalid ip address " << ipaddr << std::endl;
  }
  assert(ret != 0);

  ret = connect(s_, (struct sockaddr *)&remote_, sizeof(remote_));
  if (ret < 0) {
    XTRACE(IPC, ALW, "connect() to %s:%d failed\n", ipaddr, port);
    s_ = -1;
  }
}

int TCPClient::senddata(char *buffer, int len) {
  if (s_ < 0) {
    return -1;
  }

  if (len <= 0) {
    XTRACE(IPC, WAR, "TCPClient::senddata() no data specified\n");
    return 0;
  }
  int ret = send(s_, buffer, len, 0);
  if (ret <= 0) {
    XTRACE(IPC, WAR, "TCPClient::send() returns %d\n", ret);
  }
  return ret;
}
