/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/Log.h>
#include <efu/Launcher.h>
#include <iostream>
#include <map>
#include <thread>

void Launcher::launchThreads(std::shared_ptr<Detector> &detector) {
  auto startThreadsWithoutAffinity = [&detector]() {
    LOG(INIT, Sev::Info, "Launching threads without core affinity.");
    for (auto &ThreadInfo : detector->GetThreadInfo()) {
      LOG(INIT, Sev::Debug, "Creating new thread (id: {})", ThreadInfo.name);
      ThreadInfo.thread = std::thread(ThreadInfo.func);
    }
  };

  auto setThreadCoreAffinity = [](std::thread __attribute__((unused)) & thread,
                                  std::uint16_t __attribute__((unused)) core) {
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    LOG(INIT, Sev::Debug, "Setting thread affinity to core {}", core);

    int __attribute__((unused)) s = pthread_setaffinity_np(
        thread.native_handle(), sizeof(cpu_set_t), &cpuset);
    assert(s == 0);
#else
#pragma message("setaffinity only implemented for Linux")
    LOG(INIT, Sev::Notice, "setaffinity only implemented for Linux");
#endif
  };

  std::map<std::string, std::uint16_t> AffinityMap;
  for (auto &Affinity : ThreadCoreAffinity) {
    AffinityMap[Affinity.Name] = Affinity.Core;
  }
  if (0 == ThreadCoreAffinity.size()) {
    startThreadsWithoutAffinity();
  } else if (1 == ThreadCoreAffinity.size() and
             ThreadCoreAffinity[0].Name == "implicit_affinity") {
    LOG(INIT, Sev::Info, "Launching threads with implicit core affinity.");
    int CoreCounter = ThreadCoreAffinity[0].Core;
    for (auto &ThreadInfo : detector->GetThreadInfo()) {
      LOG(INIT, Sev::Debug, "Creating new thread (id: {})", ThreadInfo.name);
      ThreadInfo.thread = std::thread(ThreadInfo.func);
      setThreadCoreAffinity(ThreadInfo.thread, CoreCounter++);
    }
  } else {
    LOG(INIT, Sev::Info, "Launching threads with explicit core affinity.");
    for (auto &ThreadInfo : detector->GetThreadInfo()) {
      LOG(INIT, Sev::Debug, "Creating new thread (id: {})", ThreadInfo.name);
      ThreadInfo.thread = std::thread(ThreadInfo.func);
      if (1 == AffinityMap.count(ThreadInfo.name)) {
        setThreadCoreAffinity(ThreadInfo.thread, AffinityMap[ThreadInfo.name]);
      } else {
        LOG(INIT, Sev::Notice,
               "No thread core affinity information available for thread with "
               "id: {}", ThreadInfo.name);
      }
    }
  }
}
