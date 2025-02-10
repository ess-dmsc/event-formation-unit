// Copyright (C) 2018 - 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementation of misc. hardware checks
///
//===----------------------------------------------------------------------===//

#include <arpa/inet.h>
#include <cinttypes>
#include <common/debug/Log.h>
#include <cstring>
#include <efu/HwCheck.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB


bool HwCheck::ignoreInterface(char * IfName) {
  bool Ignored = true;
  for (auto & Interface : CheckedInterfaces) {
    if (strstr(IfName, Interface.c_str()) != NULL) {
      Ignored = false;
    }
  }
  return Ignored;
}

/// Checks for minimum MTU sizes by walking through the list of interfaces
/// returned by getifaddrs() of type AF_INET, which are both UP and RUNNING.
/// There is also support for ignoring certain interface name patterns to remove
/// MTU check for irrelevant interfaces such as ppp0 and docker0
bool HwCheck::checkMTU(const std::vector<std::string> &InterfaceList, bool PrintOnSuccess) {
  CheckedInterfaces = InterfaceList;
  int MatchCount{0};
  struct ifaddrs *ifaddr, *ifa;
  int n;

  if (getifaddrs(&ifaddr) == -1) {
    LOG(INIT, Sev::Error, "error getifaddrs()");
    return false;
  }

  /// Walk through linked list, maintaining head pointer so we can free list
  /// later
  for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
    if (ifa->ifa_addr == NULL) {
      continue;
    }

    if (ifa->ifa_addr->sa_family != AF_INET ||
        (ifa->ifa_flags & myflags) == 0) {
      continue;
    }


    if (ignoreInterface(ifa->ifa_name)) {
      LOG(INIT, Sev::Debug, "no checking of MTU for {}", ifa->ifa_name);
    } else {
      MatchCount++;
      if (!checkMTU(ifa->ifa_name)) {
        LOG(INIT, Sev::Warning, "MTU check failed for interface {}",
            ifa->ifa_name);
        freeifaddrs(ifaddr);
        return false;
      } else {
        if (PrintOnSuccess) {
          LOG(INIT, Sev::Info, "MTU check succeded for interface {}",
              ifa->ifa_name);
        }
      }
    }
  }
  if (MatchCount != (int)CheckedInterfaces.size()) {
    LOG(INIT, Sev::Warning, "MTU check - not all specified interfaces exist");
    return false;
  }
  freeifaddrs(ifaddr);
  return true;
}

/// Check the MTU of a single interface
bool HwCheck::checkMTU(const char *interface) {
  int s, af = AF_INET;
  struct ifreq ifr;

  if ((s = socket(af, SOCK_DGRAM, 0)) < 0) {
    LOG(INIT, Sev::Error, "error: socket");
  }

  ifr.ifr_addr.sa_family = af;
  strcpy(ifr.ifr_name, interface);
  if (ioctl(s, SIOCGIFMTU, (caddr_t)&ifr) < 0) {
    LOG(INIT, Sev::Warning, "warn: ioctl (get mtu): {}", ifr.ifr_name);
    return false;
  }

  LOG(INIT, Sev::Info, "MTU of {} is {}", interface, ifr.ifr_mtu);
  close(s);

  return ifr.ifr_mtu >= MinimumMtu;
}

/// Display interface name and family (including symbolic
/// form of the latter for the common families)
// void HwCheck::debugPrint(struct ifaddrs * ifa) {
//   int family = ifa->ifa_addr->sa_family;
//
//   printf("%-8s %s (%d) FLAGS: %08x\n",
//         ifa->ifa_name,
//         #ifdef __linux__
//         (family == AF_PACKET) ? "AF_PACKET" :
//         #else
//         (family == AF_LINK) ? "AF_LINK" :
//         #endif
//         (family == AF_INET) ? "AF_INET" :
//         (family == AF_INET6) ? "AF_INET6" : "???",
//         family,
//         ifa->ifa_flags);
// }
