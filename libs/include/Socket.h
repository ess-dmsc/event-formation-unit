/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declaration of the Socket abstration for BSD sockets
/// it is used in detector pipeline plugins for receive and in udptx for transmit
///
//===----------------------------------------------------------------------===//

#pragma once

#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <sys/socket.h>

/// BSD Socket abstractions for TCP and UDP transmitters and receivers
class Socket {
public:
  enum class type { UDP, TCP };
  static const int buflen_max = 9000;

  class Endpoint {
  public:
    const char *ipaddr;
    uint16_t port;
    Endpoint(const char *ip_address, uint16_t port_number)
        : ipaddr(ip_address), port(port_number) {}
  };

  /// Create a socker abstraction of type UDP or TCP
  Socket(Socket::type type);

  /// Attempt to specify the socket receive and transmit buffer sizes (for performance)
  int setBufferSizes(int sndbuf, int rcvbuf);

  /// Get tx and rx buffer sizes
  void getBufferSizes(int & sendBuffer, int & receiveBuffer);

  /// Print the current values for receive and trasmit buffer sizes
  void printBufferSizes(void);

  /// Set a timeout for recv() function rather than wait for ever
  int setRecvTimeout(int seconds, int usecs);

  /// Set socket option (Mac only) for not sending SIGPIPE on transmitting on invalid socket
  int setNOSIGPIPE();

  /// Specify ip address of interface to receive data on and port number to listen on
  void setLocalSocket(const char *ipaddr, int port);

  /// Specify ip address and port number of remote end
  void setRemoteSocket(const char *ipaddr, int port);

  /// Connect (TCP only) to remote endpoint
  int connectToRemote();

  /// Receive data on socket into buffer with specified length
  ssize_t receive(void *receiveBuffer, int bufferSize);

  /// Send data in buffer with specified length
  int send(void *dataBuffer, int dataLength);

  /// \brief To check is data can be transmitted or received
  bool isValidSocket();

private:
  int SocketFileDescriptor{-1};
  const char * RemoteIp;
  int RemotePort;
  struct sockaddr_in remoteSockAddr;

  /// wrapper for getsockopt() system call
  int getSockOpt(int option);

  /// wrapper for setsockopt() system call
  int setSockOpt(int option, void *value, int size);
};

/// UDP receiver only needs to specify local socket
class UDPReceiver : public Socket {
public:
  UDPReceiver(Endpoint local) : Socket(Socket::type::UDP) {
    this->setLocalSocket(local.ipaddr, local.port);
  };
};

/// UDP transmitter needs to specify both local and remote socket
class UDPTransmitter : public Socket {
public:
  UDPTransmitter(Endpoint local, Endpoint remote) : Socket(Socket::type::UDP) {
    this->setLocalSocket(local.ipaddr, local.port);
    this->setRemoteSocket(remote.ipaddr, remote.port);
  };
};

class TCPTransmitter : public Socket {
public:
  ///
  TCPTransmitter(const char *ip, int port);

  ///
  int senddata(char *buffer, int len);
};
