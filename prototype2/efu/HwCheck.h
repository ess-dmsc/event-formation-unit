/** Copyright (C) 2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class used to perform OS and Hardware related checks prior to
/// application start
///
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <vector>
#include <net/if.h>

class HwCheck {
public:
  /// status can be checked with ifconfig, here we request
  /// that the interface is both up and running meaning that
  /// there is link activity and an ip address has been assigned
  static const int myflags = IFF_UP | IFF_RUNNING;

  /// Currently the constructor does nothing
  HwCheck() {};

  /// Select interfaces to check
  bool checkMTU(std::vector<std::string> ignore);

  /// setter for MTU size check, mostly used for reverting
  /// to a lower MTU size, when running on ad hoc servers
  void setMinimumMTU(int mtu) { minimumMtu = mtu; }

  /// Gleaned from MacOS and CentOS and deemed ignore worthy
  std::vector<std::string> defaultIgnoredInterfaces = {"ppp0", "docker", "ov-"};

private:
  /// Check a single interface
  bool checkMTU(const char * interface);

  ///
  //void debugPrint(struct ifaddrs * ifa);

  /// default for Ethernet interfaces is 1500 bytes, but better performance
  /// can be achieved using larger packet sizes
  int minimumMtu{ 9000 };
};
