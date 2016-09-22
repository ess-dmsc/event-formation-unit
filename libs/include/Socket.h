/** Copyright (C) 2016 European Spallation Source */

#include <arpa/inet.h>
#include <inttypes.h>
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

  int local(const char *ipaddr, int port);
  int remote(const char *ipaddr, int port);

  int receive();
  int receive(void *buffer, int rcvlen);

  int send();

private:
  int s_{-1};
  uint16_t buflen_{buflen_max};
  struct sockaddr_in local_;
  struct sockaddr_in remote_;
  char buffer_[buflen_max];
};

class UDPServer : public Socket {
public:
  UDPServer(Endpoint local) : Socket(Socket::type::UDP) {
    this->local(local.ipaddr, local.port);
  };
};

class UDPClient : public Socket {
public:
  UDPClient(Endpoint local, struct Endpoint remote)
      : Socket(Socket::type::UDP) {
    this->local(local.ipaddr, local.port);
    this->remote(remote.ipaddr, remote.port);
  };
};
