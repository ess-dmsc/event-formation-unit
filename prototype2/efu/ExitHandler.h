/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Class to register exit handlers for program termination
 */

#pragma once
#include <common/Trace.h>
#include <stdlib.h>

class ExitHandler {
public:
  /** @brief constructor does nothing */
  ExitHandler();

private:
  /** Traces an exit message, also sends to Graylog server */
  static void signalhandler(int a);
};
