/** Copyright (C) 2016 European Spallation Source */

#pragma once

#include <arpa/inet.h>
#include <cassert>
#include <cinttypes>
#include <sys/socket.h>

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

  Socket(Socket::type type);
  int buflen(uint16_t buflen);

  int setbuffers(int sndbuf, int rcvbuf);
  void printbuffers(void);
  int settimeout(int seconds, int usecs);

  void local(const char *ipaddr, int port);
  void remote(const char *ipaddr, int port);

  int receive();
  int receive(void *buffer, int rcvlen);

  int send();                      /* send uninitialized data  (dummy)*/
  int send(void *buffer, int len); /**< send user specified data */

private:
  int s_{-1};
  uint16_t buflen_{buflen_max};
  struct sockaddr_in local_;
  struct sockaddr_in remote_;
  char buffer_[buflen_max];

  int getopt(int option);
  int setopt(int option, void *value, int size);
};

class UDPServer : public Socket {
public:
  UDPServer(Endpoint local) : Socket(Socket::type::UDP) {
    this->local(local.ipaddr, local.port);
  };
};

class UDPClient : public Socket {
public:
  UDPClient(Endpoint local, Endpoint remote) : Socket(Socket::type::UDP) {
    this->local(local.ipaddr, local.port);
    this->remote(remote.ipaddr, remote.port);
  };
};

class TCPClient {
public:
  TCPClient(const char *ip, int port);
  int senddata(char *buffer, int len);

private:
  int s_{-1};
  sockaddr_in remote_;
};
