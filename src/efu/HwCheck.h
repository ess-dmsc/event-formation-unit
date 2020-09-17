// Copyright (C) 2018-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Class used to perform OS and Hardware related checks prior to
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

  /// \brief Currently the constructor does nothing
  HwCheck() {};

  /// \brief Select interfaces to check
  bool checkMTU(std::vector<std::string> ignore, bool PrintOnSuccess = false);

  /// setter for MTU size check, mostly used for reverting
  /// to a lower MTU size, when running on ad hoc servers
  void setMinimumMTU(int mtu) { MinimumMtu = mtu; }

  /// Gleaned from MacOS and CentOS and deemed ignore worthy
  std::vector<std::string> IgnoredInterfaces = {"ppp0", "docker", "ov-", "virbr"};

  /// \brief
  // bool checkDiskSpace(std::vector<std::string> checkdirs);

  /// \brief
  // std::vector<std::string> DirectoriesToCheck = {".", "/"};

private:
  /// Check a single interface
  bool checkMTU(const char * interface);

  ///
  //void debugPrint(struct ifaddrs * ifa);

  /// default for Ethernet interfaces is 1500 bytes, but better performance
  /// can be achieved using larger packet sizes
  int MinimumMtu{ 9000 };

  /// \todo change arbitrary value to something better?
  // const uint64_t MinDiskAvailable = 30 * 1000 * 1000 * 1000ULL;

  /// \todo change arbitrary value to something better?
  // const float MinDiskPercentFree = 50.0;


};
