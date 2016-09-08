#include <arpa/inet.h>
#include <inttypes.h>
#include <sys/socket.h>

class Socket {
public:
  enum class type { UDP, TCP };

  Socket(Socket::type type, const char *ipaddr, int port);
  int Local(const char *ipaddr, int port);
  int Remote(const char *ipaddr, int port);
  int Receive();
  int Receive(void *buffer, int rcvlen);
  int Send();

private:
  int s_{-1};
  uint16_t buflen_{9000};
  struct sockaddr_in local_;
  struct sockaddr_in remote_;
  char buffer_[9000];

protected:
  Socket();
};

class UDPServer : public Socket {
public:
  UDPServer(const char *ipaddr, int port)
      : Socket(Socket::type::UDP, ipaddr, port){};
};

class UDPClient : public Socket {
public:
  UDPClient(const char *ipaddr, int port)
      : Socket(Socket::type::UDP, "0.0.0.0", 0) {
    Remote(ipaddr, port);
  };
};
