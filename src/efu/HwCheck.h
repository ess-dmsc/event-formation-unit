// Copyright (C) 2018-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Class used to perform OS and Hardware related checks prior to
/// application start
///
//===----------------------------------------------------------------------===//

#pragma once

#include <net/if.h>
#include <string>
#include <vector>

class HwCheck {
public:
  /// status can be checked with ifconfig, here we request
  /// that the interface is both up and running meaning that
  /// there is link activity and an ip address has been assigned
  static const int myflags = IFF_UP | IFF_RUNNING;

  /// \brief Currently the constructor does nothing
  HwCheck(){};

  /// \brief check if interface should be ignored
  bool ignoreInterface(char * IfName);

  /// \brief Select interfaces to check
  bool checkMTU(const std::vector<std::string> &ignore, bool PrintOnSuccess = false);

  /// setter for MTU size check, mostly used for reverting
  /// to a lower MTU size, when running on ad hoc servers
  void setMinimumMTU(int mtu) { MinimumMtu = mtu; }

  /// \brief
  // bool checkDiskSpace(const std::vector<std::string> &checkdirs);

  /// \brief
  // std::vector<std::string> DirectoriesToCheck = {".", "/"};

private:
  /// Check a single interface
  bool checkMTU(const char *interface);

  /// \brief List of interfaces that must pass
  std::vector<std::string> CheckedInterfaces{};

  /// default for Ethernet interfaces is 1500 bytes, but better performance
  /// can be achieved using larger packet sizes
  int MinimumMtu{9000};

  /// \todo change arbitrary value to something better?
  // const uint64_t MinDiskAvailable = 30 * 1000 * 1000 * 1000ULL;

  /// \todo change arbitrary value to something better?
  // const float MinDiskPercentFree = 50.0;
};
