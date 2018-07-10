
#include <string>
#include <vector>
#include <net/if.h>

#pragma once

class HwCheck {
public:
  static const int defaultMinimumMTU{9000};

  static const int myflags = IFF_UP | IFF_RUNNING;

  HwCheck(int mtu) : minimumMtu(mtu) {};

  /// Seelct interfaces to check
  bool checkMTU(std::vector<std::string> ignore);

  ///
  std::vector<std::string> defaultIgnoredInterfaces = {"ppp0", "docker", "ov-"};

private:
  /// Check a single interface
  bool checkMTU(const char * interface);

  ///
  void debugPrint(struct ifaddrs * ifa);

  ///
  int minimumMtu{defaultMinimumMTU};


};
