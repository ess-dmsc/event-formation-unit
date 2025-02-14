// Copyright (C) 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Log functions for the ESS logger
//===----------------------------------------------------------------------===//

#include "Log.h"

#ifdef UNIT_TEST

MockLogger *MockLogger::instance = nullptr; // Definition and initialization

void mockLogFunction(const std::string &category, const std::string &message) {
  MockLogger *ptrToInstance = MockLogger::instance;
  if (ptrToInstance) {
    ptrToInstance->log(category, message);
  }
}
#endif
