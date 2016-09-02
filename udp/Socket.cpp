#include <Socket.h>
#include <stdio.h>
#include <string.h>

Socket::Socket(uint16_t port, uint16_t buflen)
{
    mPort = port;
    mBufLen = buflen;

    //create a UDP socket
    if ((mSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("socket() failed");
    }

    // zero out the structure
    memset( (char *) &mSiLocal, 0, sizeof(mSiLocal));

    mSiLocal.sin_family = AF_INET;
    mSiLocal.sin_port = htons(port);
    mSiLocal.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind socket to port
    if ( bind(mSocket , (struct sockaddr*)&mSiLocal, sizeof(mSiLocal)) == -1)
    {
        printf("bind() failed\n");
    }
}

/** */
int Socket::Receive()
{
  int recv_len;
  socklen_t slen;
  //try to receive some data, this is a blocking call
  if ((recv_len = recvfrom(mSocket, mBuffer, mBufLen, 0, (struct sockaddr *) &mSiRemote, &slen)) < 0) {
    printf("Receive() failed\n");
    return 0;
  }
  return recv_len;
}
