// Copyright (C) 2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Helper classes for Timepix3 tests
//===-----------

#include <gtest/gtest.h>
#include <modules/timepix3/dataflow/DataObserverTemplate.h>

#pragma once

template <typename DataEvent>
class DataEventTestHandler
    : public Observer::DataEventObserver<DataEvent> {
private:
  std::unique_ptr<DataEvent> testData;

public:
  void setData(const DataEvent &appliedData) {
    testData = std::make_unique<DataEvent>(appliedData);
  }

  void applyData(const DataEvent &appliedData) override {
    EXPECT_EQ(appliedData, *testData);
  };
};
