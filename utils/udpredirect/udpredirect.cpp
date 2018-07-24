/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc!=3 && argc!=5) {
        printf("Usage: %s our-ip our-port send-to-ip send-to-port\n",argv[0]);
        printf("Usage: %s our-ip our-port             # echo mode\n",argv[0]);
        exit(1);
    }

    int os=socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    int sendsocket =socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

    struct sockaddr_in a;

    memset(&a, 0, sizeof(a));
    a.sin_family=AF_INET;
    a.sin_addr.s_addr=inet_addr(argv[1]);
    a.sin_port=htons(atoi(argv[2]));

    if( bind(os, (struct sockaddr *)&a, sizeof(a)) == -1) {
        printf("Can't bind our address (%s:%s)\n", argv[1], argv[2]);
        exit(1);
    }

    struct sockaddr_in destAddr;
    socklen_t destAddrLen = sizeof(destAddr);
    memset(&destAddr, 0, sizeof(destAddr));
    if(argc==5) {
        printf("Setting output ip address and port number\n");
        destAddr.sin_family=AF_INET;
        destAddr.sin_addr.s_addr = inet_addr(argv[3]);
        destAddr.sin_port = htons(atoi(argv[4]));
    }

    char buf[65535];
    struct sockaddr_in sa;
    socklen_t sn = sizeof(sa);
    memset(&sa, 0, sizeof(sa));
    while(1) {
        int n = recvfrom(os, buf, sizeof(buf), 0, (struct sockaddr *)&sa, &sn);
        printf("got data of len: %d\n", n);

        if(n<=0) {
            continue;
        }

        if(argc==3) {
            sendto(os, buf, n, 0, (struct sockaddr *)&sa, sn);
            printf("Case A: echo mode\n");
        } else {
            printf("Normal case?\n");
            int m = sendto(sendsocket, buf, n, 0, (struct sockaddr *)&destAddr, destAddrLen);
            if (m < 0) {
                perror("sendto():");
            }
            printf("sendto returns %d\n", m);
        }
    }
}
