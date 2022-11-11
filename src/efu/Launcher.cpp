// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of thread launcher
///
//===----------------------------------------------------------------------===//

#include <cassert>
#include <common/debug/Log.h>
#include <common/detector/Detector.h>
#include <common/detector/EFUArgs.h>
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

  startThreadsWithoutAffinity();
}
