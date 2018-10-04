/** Copyright (C) 2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class used to perform OS and Hardware related checks prior to
/// application start
///
//===----------------------------------------------------------------------===//

#include <efu/HwCheck.h>
#include <arpa/inet.h>
#include <common/Log.h>
#include <cinttypes>
#include <cstring>
#include <ifaddrs.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <unistd.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

/// Checks for minimum MTU sizes by walking through the list of interfaces returned
/// by getifaddrs() of type AF_INET, which are both UP and RUNNING. There is also
/// support for ignoring certain interface name patterns to remove MTU check for
/// irrelevant interfaces such as ppp0 and docker0
bool HwCheck::checkMTU(std::vector<std::string> ignore) {
  struct ifaddrs *ifaddr, *ifa;
  int n;

  if (getifaddrs(&ifaddr) == -1) {
    LOG(INIT, Sev::Error, "error getifaddrs()");
     return false;
  }

  /// Walk through linked list, maintaining head pointer so we can free list later
  for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
    if (ifa->ifa_addr == NULL) {
      continue;
    }

    if (ifa->ifa_addr->sa_family != AF_INET || (ifa->ifa_flags & myflags) == 0) {
      continue;
    }

    bool tobeignored = false;
    for (auto & ignorePattern : ignore) {
      if (strstr(ifa->ifa_name, ignorePattern.c_str()) != NULL) {
        tobeignored = true;
      }
    }

    if (tobeignored) {
      LOG(INIT, Sev::Debug, "no checking of MTU for {}", ifa->ifa_name);
    } else {
      if (!checkMTU(ifa->ifa_name)) {
        freeifaddrs(ifaddr);
        return false;
      }
    }
  }
  freeifaddrs(ifaddr);
  return true;
}

/// Check the MTU of a single interface
bool HwCheck::checkMTU(const char * interface) {
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


bool HwCheck::checkDiskSpace(std::vector<std::string> directories) {
   bool ok = true;
   for (auto file : directories) {
     struct statvfs fsstats;
     int ret = statvfs(file.c_str(), &fsstats);
     if (ret < 0) {
       LOG(INIT, Sev::Info, "Diskcheck ignoring file/dir {}", file);
       continue;;
    }

    std::string passfail = "passed";
    uint64_t bytes_avail = fsstats.f_frsize * fsstats.f_bavail;
    float percent_avail = fsstats.f_blocks == 0 ?
           0.0 :(fsstats.f_bavail * 100.0) / fsstats.f_blocks;
    if ( (bytes_avail < MinDiskAvailable) or (percent_avail < MinDiskPercentFree) ) {
      ok = false;
      passfail = "failed";
    }
    LOG(INIT, Sev::Warning, "Diskcheck {} for {}: available: {}B ({}%)", passfail, file, bytes_avail, percent_avail);
  }

  return ok;
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
