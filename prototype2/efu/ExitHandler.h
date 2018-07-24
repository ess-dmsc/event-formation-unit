/// Copyright (C) 2016-2018 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Class to register exit handlers for program termination
///
//===----------------------------------------------------------------------===//

#pragma once
#include <common/Trace.h>
#include <stdlib.h>

class ExitHandler {
public:
  enum class Exit {
    NoExit,
    Exit,
  };
  /** \brief constructor does nothing */
  static void InitExitHandler();

  static Exit HandleLastSignal();

private:
  /** noncritical signals (Ctrl-C), stop threads nicely */
  static void nonCritical(int a);

  /** Critical signals, immediate exit */
  static void critical(int a);

  /** print a stack trace */
  static void printTrace(void);
};
