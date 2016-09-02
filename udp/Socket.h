#include <arpa/inet.h>
#include <inttypes.h>
#include <sys/socket.h>

class Socket {
public:
  Socket(uint16_t port, uint16_t buflen);
  int Receive();

private:
  int mSocket{-1};
  uint16_t mPort;
  uint16_t mBufLen;
  struct sockaddr_in mSiLocal;
  struct sockaddr_in mSiRemote;
  char mBuffer[2000];
};
