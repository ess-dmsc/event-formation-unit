// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file contains the declaration of the Socket abstraction for BSD
/// socket system calls
///
/// Used in detector pipeline plugins for receive udp unicast and multicast and
/// in efu application for transmitting Grafana stats and receiving TCP commands
//===----------------------------------------------------------------------===//

#pragma once

#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <netinet/ip.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

/// BSD Socket abstractions for TCP and UDP transmitters and receivers
class Socket {
public:
  enum class SocketType { UDP, TCP };

  class Endpoint {
  public:
    const std::string IpAddress;
    uint16_t Port;
    Endpoint(const std::string ip_address, uint16_t port_number)
        : IpAddress(ip_address), Port(port_number) {}
  };

  /// \brief Is this a dotted quad ip address?
  /// Valid addresses must be of the form 'a.b.d.c' where
  /// a-d can range from 0 to 255
  static bool isValidIp(const std::string &ipAddress);

  /// \brief Return dotted quad by resolving hostname
  /// Essentially a wrapper for gethostbyname() returning
  /// the first entry in the ip address table
  static std::string getHostByName(std::string &name);

  /// Create a socker abstraction of type UDP or TCP
  Socket(Socket::SocketType Type);

  /// Close the file descriptor
  ~Socket() {
    if (SocketFileDescriptor >= 0) {
      close(SocketFileDescriptor);
    }
  }

  /// Set TTL to 1 for IP multicast (for transmitters)
  void setMulticastTTL();

  /// Allow reuse of ip and port, send igmp membership info
  // void setMulticastReceive(const std::string &MultiCastAddress);
  void setMulticastReceive();

  /// Attempt to specify the socket receive and transmit buffer sizes (for
  /// performance)
  int setBufferSizes(int sndbuf, int rcvbuf);

  /// Get tx and rx buffer sizes
  void getBufferSizes(int &sendBuffer, int &receiveBuffer);

  /// Check that buffer sizes meet expectations
  void checkRxBufferSizes(std::int32_t MinRxBufferSize);

  /// Print the current values for receive and transmit buffer sizes
  void printBufferSizes(void);

  /// Set a timeout for recv() function rather than wait for ever
  int setRecvTimeout(int seconds, int usecs);

  /// Set socket option (Mac only) for not sending SIGPIPE on transmitting on
  /// invalid socket
  int setNOSIGPIPE();

  /// Specify ip address of interface to receive data on and port number to
  /// listen on
  void setLocalSocket(const std::string ipaddr, int port);

  /// Specify ip address and port number of remote end
  void setRemoteSocket(const std::string ipaddr, int port);

  /// Connect (TCP only) to remote endpoint
  int connectToRemote();

  /// Receive data on socket into buffer with specified length
  ssize_t receive(void *receiveBuffer, int bufferSize);

  /// Send data in buffer with specified length
  int send(void const *dataBuffer, int dataLength);

  /// \brief To check if data can be transmitted or received
  bool isValidSocket();

  /// \brief Check if address is IP multicast (Class 'D')
  static bool isMulticast(const std::string &IpAddress) {
    return IN_MULTICAST(ntohl(inet_addr(IpAddress.c_str())));
  };

private:
  int SocketFileDescriptor{-1};
  bool SocketIsGood{true};
  int SockOptFlagOn{1};
  struct ip_mreq MulticastRequest;
  std::string RemoteIp;
  int RemotePort;
  struct sockaddr_in remoteSockAddr;
  struct sockaddr_in localSockAddr;

  /// wrapper for getsockopt() system call
  int getSockOpt(int option);

  /// wrapper for setsockopt() system call
  int setSockOpt(int option, void *value, int size);
};

/// UDP receiver only needs to specify local socket
class UDPReceiver : public Socket {
public:
  UDPReceiver(Endpoint Local) : Socket(Socket::SocketType::UDP) {
    this->setLocalSocket(Local.IpAddress, Local.Port);
  };
};

/// UDP transmitter needs to specify both local and remote socket
class UDPTransmitter : public Socket {
public:
  UDPTransmitter(Endpoint Local, Endpoint Remote)
      : Socket(Socket::SocketType::UDP) {
    this->setLocalSocket(Local.IpAddress, Local.Port);
    this->setRemoteSocket(Remote.IpAddress, Remote.Port);
  };
};

class TCPTransmitter : public Socket {
public:
  ///
  TCPTransmitter(const std::string ip, int port);

  ///
  int senddata(char const *buffer, int len);
};
