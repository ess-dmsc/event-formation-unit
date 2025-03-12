// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Log functions for the ESS logger
//===----------------------------------------------------------------------===//

#include "Log.h"

#ifdef UNIT_TEST

MockLogger *MockLogger::currentMockLogger = nullptr;

void MockLogger::mockLogFunction(const std::string category,
                                 const std::string message) {
  if (currentMockLogger) {
    currentMockLogger->log(category, message);
  }
}
#endif
