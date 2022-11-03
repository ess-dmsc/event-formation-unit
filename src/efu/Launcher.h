// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for launching processing threads
///
//===----------------------------------------------------------------------===//

#pragma once
#include <common/detector/Detector.h>
#include <common/detector/EFUArgs.h>
#include <vector>

class Launcher {
public:
  /// \brief Launches previously Loaded detector functions
  /// \param ld Dynamic detector object (from Loader)
  /// \param args Arguments to be passed to threads
  /// \param cpus vector of three cpuids for launching input, processing and
  /// output threads.
  Launcher(){}

  void launchThreads(std::shared_ptr<Detector> &detector);

private:
};
