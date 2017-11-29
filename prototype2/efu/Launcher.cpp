/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cassert>
#include <common/Detector.h>
#include <common/EFUArgs.h>
#include <common/Trace.h>
#include <efu/Launcher.h>
#include <iostream>
#include <thread>
#include <map>

void Launcher::launchThreads(std::shared_ptr<Detector> &detector) {
  auto startThreadsWithoutAffinity = [&detector]() {
    XTRACE(MAIN, ALW, "Launching threads without core affinity.\n");
    for (auto &ThreadInfo : detector->GetThreadInfo()) {
      XTRACE(MAIN, ALW, "Creating new thread (id: %s)\n", ThreadInfo.name.c_str());
      ThreadInfo.thread = std::thread(ThreadInfo.func);
    }
  };
  
  auto setThreadCoreAffinity = [](std::thread &thread, std::uint16_t core) {
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    XTRACE(MAIN, ALW, "Setting thread affinity to core %d\n", core);
    GLOG_INF("Setting thread affinity to core " + std::to_string(core));
      
      int __attribute__((unused))s = pthread_setaffinity_np(thread.native_handle(), sizeof(cpu_set_t), &cpuset);
      assert(s == 0);
    }
#else
#pragma message("setaffinity only implemented for Linux")
    GLOG_WAR("setaffinity only implemented for Linux");
#endif
  };

  std::map<std::string,std::uint16_t> AffinityMap;
  for (auto &Affinity : ThreadCoreAffinity) {
    AffinityMap[Affinity.Name] = Affinity.Core;
  }
  if (0 == ThreadCoreAffinity.size()) {
    startThreadsWithoutAffinity();
  } else if (1 == ThreadCoreAffinity.size() and ThreadCoreAffinity[0].Name == "implicit_affinity") {
    XTRACE(MAIN, ALW, "Launching threads with implicit core affinity.\n");
    int CoreCounter = ThreadCoreAffinity[0].Core;
    for (auto &ThreadInfo : detector->GetThreadInfo()) {
      XTRACE(MAIN, ALW, "Creating new thread (id: %s)\n", ThreadInfo.name.c_str());
      ThreadInfo.thread = std::thread(ThreadInfo.func);
      setThreadCoreAffinity(ThreadInfo.thread, CoreCounter++);
    }
  } else {
    XTRACE(MAIN, ALW, "Launching threads with explicit core affinity.\n");
    for (auto &ThreadInfo : detector->GetThreadInfo()) {
      XTRACE(MAIN, ALW, "Creating new thread (id: %s)\n", ThreadInfo.name.c_str());
      ThreadInfo.thread = std::thread(ThreadInfo.func);
      if (1 == AffinityMap.count(ThreadInfo.name)) {
        setThreadCoreAffinity(ThreadInfo.thread, AffinityMap[ThreadInfo.name]);
      }
    }
  }
}
