#include <Socket.h>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;

Socket::Socket(Socket::type stype) {
  auto type = (stype == Socket::type::UDP) ? SOCK_DGRAM : SOCK_STREAM;
  auto proto = (stype == Socket::type::UDP) ? IPPROTO_UDP : IPPROTO_TCP;

  if ((s_ = socket(AF_INET, type, proto)) == -1) {
    cout << "socket() failed" << endl;
    exit(1);
  }
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

int Socket::local(const char *ipaddr, int port) {
  // zero out the structures
  memset((char *)&local_, 0, sizeof(local_));
  local_.sin_family = AF_INET;
  local_.sin_port = htons(port);
  inet_aton(ipaddr, &local_.sin_addr);

  // bind socket to port
  return bind(s_, (struct sockaddr *)&local_, sizeof(local_));
}

int Socket::remote(const char *ipaddr, int port) {
  // zero out the structures
  memset((char *)&remote_, 0, sizeof(remote_));
  remote_.sin_family = AF_INET;
  remote_.sin_port = htons(port);
  return inet_aton(ipaddr, &remote_.sin_addr);
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
  int recv_len;
  socklen_t slen = 0;
  // try to receive some data, this is a blocking call
  if ((recv_len = recvfrom(s_, buffer, buflen, 0, (struct sockaddr *)&remote_,
                           &slen)) < 0) {
    printf("Receive() failed: %d (sockt fd %d)\n", recv_len, s_);
    perror("recvfrom: ");
    return 0;
  }
  return recv_len;
}
