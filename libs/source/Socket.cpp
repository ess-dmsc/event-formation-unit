/** Copyright (C) 2016 European Spallation Source */

#include <Socket.h>
#include <cstdlib>
#include <cstring>
#include <iostream>

using namespace std;

Socket::Socket(Socket::type stype) {
  auto type = (stype == Socket::type::UDP) ? SOCK_DGRAM : SOCK_STREAM;
  auto proto = (stype == Socket::type::UDP) ? IPPROTO_UDP : IPPROTO_TCP;

  if ((s_ = socket(AF_INET, type, proto)) == -1) {
    cout << "socket() failed" << endl;
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
  cout << "Socket rcv buffer size: " << getopt(SO_RCVBUF) << endl;
  cout << "Socket snd buffer size: " << getopt(SO_SNDBUF) << endl;
}

int Socket::settimeout(int seconds) {
  struct timeval timeout;
  timeout.tv_sec = seconds;
  timeout.tv_usec = 0;
  return setopt(SO_RCVTIMEO, &timeout, sizeof(timeout));
}

int Socket::buflen(uint16_t buflen) {
  if (buflen > buflen_max) {
    cout << "Specified buffer length " << buflen << " too large, adjusted to "
         << buflen_max << endl;
    buflen_ = buflen_max;
  } else {
    buflen_ = buflen;
  }
  return buflen_;
}

void Socket::local(const char *ipaddr, int port) {
  // zero out the structures
  std::memset((char *)&local_, 0, sizeof(local_));
  local_.sin_family = AF_INET;
  local_.sin_port = htons(port);
  inet_aton(ipaddr, &local_.sin_addr);

  // bind socket to port
  auto ret = bind(s_, (struct sockaddr *)&local_, sizeof(local_));
  if (ret != 0) {
    cout << "bind failed - is port " << port << " already in use?" << endl;
  }
  assert(ret == 0);
}

void Socket::remote(const char *ipaddr, int port) {
  // zero out the structures
  std::memset((char *)&remote_, 0, sizeof(remote_));
  remote_.sin_family = AF_INET;
  remote_.sin_port = htons(port);
  auto ret = inet_aton(ipaddr, &remote_.sin_addr);
  if (ret == 0) {
    cout << "invalid ip address " << ipaddr << endl;
  }
  assert(ret != 0);
}

int Socket::send() {
  int ret = sendto(s_, buffer_, buflen_, 0, (struct sockaddr *)&remote_,
                   sizeof(remote_));
  if (ret < 0) {
    cout << "unable to send on socket" << endl;
    perror("sendto");
    exit(1);
  }

  return ret;
}

int Socket::send(void *buffer, int len) {
  int ret =
      sendto(s_, buffer, len, 0, (struct sockaddr *)&remote_, sizeof(remote_));
  if (ret < 0) {
    cout << "unable to send on socket" << endl;
    perror("sendto");
    exit(1);
  }

  return ret;
}

/** */
int Socket::receive() {
  int recv_len;
  socklen_t slen = 0;
  // try to receive some data, this is a blocking call
  if ((recv_len = recvfrom(s_, buffer_, buflen_, 0, (struct sockaddr *)&remote_,
                           &slen)) < 0) {
    printf("Receive() failed: %d (sockt fd %d)\n", recv_len, s_);
    perror("recvfrom: ");
    return 0;
  }
  return recv_len;
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
    cout << "getsockopt() failed" << endl;
    return ret;
  }
  return optval;
}

int Socket::setopt(int option, void *value, int size) {
  int ret;
  if ((ret = setsockopt(s_, SOL_SOCKET, option, value, size)) < 0) {
    cout << "setsockopt() failed" << endl;
  }
  return ret;
}
