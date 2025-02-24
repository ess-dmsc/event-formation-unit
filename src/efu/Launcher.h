// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class for launching processing threads
///
/// The Launcher class is responsible for launching and managing threads
/// for processing data using a dynamic detector object. It takes care of
/// initializing the threads with the necessary arguments and handling any
/// exceptions that may occur during their execution.
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/detector/EFUArgs.h>
#include <functional>
#include <vector>

///
/// \class Launcher
///
/// \brief Launches previously loaded detector functions
///
/// This class provides functionality to launch threads for input, processing,
/// and output using a dynamic detector object. It ensures that the threads
/// are properly initialized and managed.
///
class Launcher {
public:

  ///
  /// \brief Constructor for the Launcher class
  /// \param keep_running Reference to an integer that controls the running
  /// state
  ///
  Launcher(int &keep_running) : KeepRunningRef(keep_running) {}

  ///
  /// \brief Launches the threads for the detector
  /// \param detector Shared pointer to the detector object
  ///
  void launchThreads(std::shared_ptr<Detector> &detector);

private:
  int &KeepRunningRef; ///< Reference to a flag of main loop to keep running

  ///
  /// \brief Wrapper function for handling exceptions in threads
  ///        and initiating graceful shutdown in case of unhandled exceptions
  /// \param ThreadInfo Information about the thread
  ///
  void exceptionHandlingWrapper(ThreadInfo &ThreadInfo);
};
