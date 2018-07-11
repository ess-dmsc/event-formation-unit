#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if.h>

#define MYFLAGS (IFF_UP || IFF_RUNNING)


int main(int argc, char * argv[]) {
     struct ifaddrs *ifaddr, *ifa;
       int family, s, n;

       if (getifaddrs(&ifaddr) == -1) {
           printf("error getifaddrs()\n");
           return -1;
       }

       /* Walk through linked list, maintaining head pointer so we
          can free list later */

       for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
           if (ifa->ifa_addr == NULL)
               continue;

           family = ifa->ifa_addr->sa_family;

           if (family != AF_INET) {
               continue;
           }

           if ((ifa->ifa_flags & MYFLAGS) == 0) {
               continue;
           }

           /* Display interface name and family (including symbolic
              form of the latter for the common families) */

           printf("%-8s %s (%d) FLAGS: %08x\n",
                  ifa->ifa_name,
                  #ifdef __linux__
                  (family == AF_PACKET) ? "AF_PACKET" :
                  #else
                  (family == AF_LINK) ? "AF_LINK" :
                  #endif
                  (family == AF_INET) ? "AF_INET" :
                  (family == AF_INET6) ? "AF_INET6" : "???",
                  family, 
                  ifa->ifa_flags);
        }
}
