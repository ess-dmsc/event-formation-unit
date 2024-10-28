// Copyright (C) 2016-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation of the Launcher class
///
//===----------------------------------------------------------------------===//

#include <efu/MainProg.h>
#include <cassert>
#include <common/debug/Log.h>
#include <common/detector/Detector.h>
#include <common/detector/EFUArgs.h>
#include <efu/Launcher.h>
#include <iostream>
#include <map>
#include <thread>

void Launcher::launchThreads(std::shared_ptr<Detector> &detector) {
  auto startThreadsWithoutAffinity = [&detector, this]() {
    LOG(INIT, Sev::Info, "Launching threads without core affinity.");
    for (auto &ThreadInfo : detector->GetThreadInfo()) {

      LOG(INIT, Sev::Debug, "Creating new thread (id: {})", ThreadInfo.name);

      /// start thread with exception handling
      ThreadInfo.thread = std::thread([this, &ThreadInfo]() { exceptionHandlingWrapper(ThreadInfo); });
    }
  };

  startThreadsWithoutAffinity();
}

void Launcher::exceptionHandlingWrapper(ThreadInfo &ThreadInfo) {
  try {
    ThreadInfo.func();
  } catch (const std::exception &e) {
    LOG(INIT, Sev::Critical, "Initiate shutdown. Thread {} threw an unhandled exception: {}", ThreadInfo.name, e.what());
    KeepRunningRef = 0;
  }
}
