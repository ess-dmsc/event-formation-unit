// Copyright (C) 2022 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Wrapper for EFU main application
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/Detector.h>
#include <common/detector/EFUArgs.h>
#include <common/time/Timer.h>
#include <efu/Graylog.h>
#include <efu/HwCheck.h>

class MainProg {
public:

  /// \brief initialise by passing the instrument name and the arguments
  /// cannot return but uses exit() instead.
  /// constructor - roughly this is first half of the old main() function
  MainProg(std::string instrument, int argc, char * argv[]);

  /// \brief setup exithandlers, launch detector - roughly equivalent to the
  /// second half of the old main()
  int run(Detector * detector);

public:
  static constexpr uint64_t MicrosecondsPerSecond{1000000};
  BaseSettings DetectorSettings;
  std::shared_ptr<Detector> detector;
  std::string DetectorName;
  Graylog graylog;
  HwCheck hwcheck;
  Timer RunTimer;
  EFUArgs Args;
};
