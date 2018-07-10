///
///

#include <HwCheck.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

bool HwCheck::checkMTU(std::vector<std::string> ignore) {
  struct ifaddrs *ifaddr, *ifa;
  int family, s, n;

  if (getifaddrs(&ifaddr) == -1) {
     printf("error getifaddrs()\n");
     return false;
  }

  /// Walk through linked list, maintaining head pointer so we
  /// can free list later
  for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
    if (ifa->ifa_addr == NULL) {
      continue;
    }

    //debugPrint(ifa);

    if (ifa->ifa_addr->sa_family != AF_INET || (ifa->ifa_flags & myflags) == 0) {
      continue;
    }



    for (auto & ignorePattern : ignore) {
      if (strstr(ifa->ifa_name, ignorePattern.c_str()) != NULL) {
        printf("no checking of MTU for %s\n", ifa->ifa_name);
        break;
      } else {
        printf("checking MTU for %s\n", ifa->ifa_name);
        if (!checkMTU(ifa->ifa_name)) {
          return false;
        }
      }
   }
  }
  return true;
}

///
bool HwCheck::checkMTU(const char * interface) {
  int s, af = AF_INET;
	struct ifreq ifr;

	if ((s = socket(af, SOCK_DGRAM, 0)) < 0) {
		printf("error: socket\n");
  }

	ifr.ifr_addr.sa_family = af;
	strcpy(ifr.ifr_name, interface);
	if (ioctl(s, SIOCGIFMTU, (caddr_t)&ifr) < 0) {
		printf("warn: ioctl (get mtu): %s\n", ifr.ifr_name);
    return false;
  }

	fprintf(stdout, "MTU of %s is %d\n", interface, ifr.ifr_mtu);
	close(s);

  return ifr.ifr_mtu >= minimumMtu;
}


/// Display interface name and family (including symbolic
/// form of the latter for the common families)
void HwCheck::debugPrint(struct ifaddrs * ifa) {
  int family = ifa->ifa_addr->sa_family;

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


int main(int argc, char * argv []) {
  HwCheck hwcheck(9000);

  std::vector<std::string> ignore = {"ppp0"};

  if (!hwcheck.checkMTU(ignore)) {
    printf("MTU check failed\n");
  }
}
