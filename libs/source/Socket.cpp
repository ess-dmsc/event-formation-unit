/** Copyright (C) 2016 European Spallation Source */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <libs/include/Socket.h>
#include <prototype2/common/Log.h>
#include <prototype2/common/Trace.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

/// \brief Use MSG_SIGNAL on Linuxes
#ifdef MSG_NOSIGNAL
#define SEND_FLAGS MSG_NOSIGNAL
#else
#define SEND_FLAGS 0
#endif

Socket::Socket(Socket::type stype) {
  auto type = (stype == Socket::type::UDP) ? SOCK_DGRAM : SOCK_STREAM;
  auto proto = (stype == Socket::type::UDP) ? IPPROTO_UDP : IPPROTO_TCP;

  if ((SocketFileDescriptor = socket(AF_INET, type, proto)) == -1) {
    LOG(IPC, Sev::Error, "socket() failed");
    exit(1);
  }
}

int Socket::setBufferSizes(int sndbuf, int rcvbuf) {
  int ok{true};
  int res{0};
  if (sndbuf) {
    if ((res = setSockOpt(SO_SNDBUF, &sndbuf, sizeof(sndbuf))) < 0) {
      ok = false;
    }
  }
  if (rcvbuf) {
    if ((res = setSockOpt(SO_RCVBUF, &rcvbuf, sizeof(rcvbuf))) < 0) {
      ok = false;
    }
  }
  if (!ok) {
    return -1;
  } else {
    return 0;
  }
}

void Socket::getBufferSizes(int & sendBuffer, int & receiveBuffer) {
  sendBuffer = getSockOpt(SO_SNDBUF);
  receiveBuffer = getSockOpt(SO_RCVBUF);
}

void Socket::printBufferSizes(void) {
  LOG(IPC, Sev::Info, "Socket receive buffer size: {}", getSockOpt(SO_RCVBUF));
  LOG(IPC, Sev::Info, "Socket send buffer size: {}", getSockOpt(SO_SNDBUF));
}

int Socket::setRecvTimeout(int seconds, int usecs) {
  struct timeval timeout;
  timeout.tv_sec = seconds;
  timeout.tv_usec = usecs;
  return setSockOpt(SO_RCVTIMEO, &timeout, sizeof(timeout));
}

int Socket::setNOSIGPIPE() {
#ifdef SYSTEM_NAME_DARWIN
    LOG(IPC, Sev::Info, "setsockopt() - MacOS specific");
    int on = 1;
    int ret = setSockOpt(SO_NOSIGPIPE, &on, sizeof(on));
    if (ret != 0) {
        LOG(IPC, Sev::Warning, "Cannot set SO_NOSIGPIPE for socket");
        perror("setsockopt():");
    }
    assert(ret ==0);
    return ret;
#else
    return 0;
#endif
}

void Socket::setLocalSocket(const char *ipaddr, int port) {
  // zero out the structures
  struct sockaddr_in localSockAddr;
  std::memset((char *)&localSockAddr, 0, sizeof(localSockAddr));
  localSockAddr.sin_family = AF_INET;
  localSockAddr.sin_port = htons(port);

  int ret = inet_aton(ipaddr, &localSockAddr.sin_addr);
  if (ret == 0) {
    std::cout << "invalid ip address " << ipaddr << std::endl;
  }
  assert(ret != 0);

  // bind socket to port
  ret = bind(SocketFileDescriptor, (struct sockaddr *)&localSockAddr, sizeof(localSockAddr));
  if (ret != 0) {
    std::cout << "bind failed - is port " << port << " already in use?"
              << std::endl;
  }
  assert(ret == 0);
}

void Socket::setRemoteSocket(const char *ipaddr, int port) {
  RemoteIp = ipaddr;
  RemotePort = port;
  // zero out the structures
  std::memset((char *)&remoteSockAddr, 0, sizeof(remoteSockAddr));
  remoteSockAddr.sin_family = AF_INET;
  remoteSockAddr.sin_port = htons(port);

  int ret = inet_aton(ipaddr, &remoteSockAddr.sin_addr);
  if (ret == 0) {
    std::cout << "invalid ip address " << ipaddr << std::endl;
  }
  assert(ret != 0);
}

int Socket::connectToRemote() {
  // zero out the structures
  struct sockaddr_in remoteSockAddr;
  std::memset((char *)&remoteSockAddr, 0, sizeof(remoteSockAddr));
  remoteSockAddr.sin_family = AF_INET;
  remoteSockAddr.sin_port = htons(RemotePort);
  int ret = inet_aton(RemoteIp, &remoteSockAddr.sin_addr);
  if (ret == 0) {
    std::cout << "invalid ip address " << RemoteIp << std::endl;
  }
  assert(ret != 0);
  ret = connect(SocketFileDescriptor, (struct sockaddr *)&remoteSockAddr, sizeof(remoteSockAddr));
  if (ret < 0) {
    LOG(IPC, Sev::Error, "connect() to {}:{} failed", RemoteIp, RemotePort);
    SocketFileDescriptor = -1;
  }
  return ret;
}

int Socket::send(void *buffer, int len) {
  XTRACE(IPC, DEB, "Socket::send(), length %d bytes", len);
  int ret =
      sendto(SocketFileDescriptor, buffer, len, SEND_FLAGS, (struct sockaddr *)&remoteSockAddr, sizeof(remoteSockAddr));
  if (ret < 0) {
    SocketFileDescriptor = -1;
    XTRACE(IPC, DEB, "sendto() failed with code %d", ret);
  }

  return ret;
}

/** */
int Socket::receive(void *buffer, int buflen) {
  socklen_t slen = 0;
  // try to receive some data, this is a blocking call
  return recvfrom(SocketFileDescriptor, buffer, buflen, 0, (struct sockaddr *)&remoteSockAddr, &slen);
}

//
// Private methods
//

int Socket::getSockOpt(int option) {
  int optval, ret;
  socklen_t optlen;
  optlen = sizeof(optval);
  if ((ret = getsockopt(SocketFileDescriptor, SOL_SOCKET, option, (void *)&optval, &optlen)) <
      0) {
    std::cout << "getsockopt() failed" << std::endl;
    return ret;
  }
  return optval;
}

int Socket::setSockOpt(int option, void *value, int size) {
  int ret;
  if ((ret = setsockopt(SocketFileDescriptor, SOL_SOCKET, option, value, size)) < 0) {
    std::cout << "setsockopt() failed" << std::endl;
  }
  return ret;
}

bool Socket::isValidSocket() {
  return (SocketFileDescriptor >= 0);
}

///
///
///
TCPTransmitter::TCPTransmitter(const char *ipaddr, int port) : Socket(Socket::type::TCP) {
  setRemoteSocket(ipaddr, port);
  setNOSIGPIPE();
  connectToRemote();
}

int TCPTransmitter::senddata(char *buffer, int len) {
  if (!isValidSocket()) {
    XTRACE(IPC, WAR, "No file descriptor for TCP transmitter");
    return -1;
  }

  if (len <= 0) {
    LOG(IPC, Sev::Warning, "TCPClient::senddata() no data specified");
    return 0;
  }

  return send(buffer, len);
}
