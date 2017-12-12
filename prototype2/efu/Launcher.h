/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class for launching processing threads
 */

#pragma once
#include <common/Detector.h>
#include <vector>
#include <common/EFUArgs.h>

class Launcher {
public:
  /** @brief Launches previously Loaded detector functions
   *  @param ld Dynamic detector object (from Loader)
   *  @param args Arguments to be passed to threads
   *  @param cpus vector of three cpuids for launching input, processing and
   *  output threads.
   */
  Launcher(std::vector<ThreadCoreAffinitySetting> ThreadAffinity) : ThreadCoreAffinity(ThreadAffinity) {};
  
  void launchThreads(std::shared_ptr<Detector> &detector);

private:
  std::vector<ThreadCoreAffinitySetting> ThreadCoreAffinity;
};
