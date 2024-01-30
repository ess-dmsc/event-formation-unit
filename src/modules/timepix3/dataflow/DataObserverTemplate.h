// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simple implementation for the observer pattern with template
/// observable
//===----------------------------------------------------------------------===//

#pragma once

#include <algorithm>
#include <cstdint>
#include <locale>
#include <memory>
#include <vector>

namespace Observer {

template <typename DataEvent> class DataEventObserver {
public:
  virtual void applyData(const DataEvent &event) = 0;
};

template <typename DataEvent> class DataEventObservable {
private:
  std::vector<DataEventObserver<DataEvent> *> dataEventListeners;

public:
  DataEventObservable<DataEvent>(){};

  virtual ~DataEventObservable<DataEvent>() = default;

  inline void subscribe(DataEventObserver<DataEvent> *listener) {

    // Only add listener if it is not already subscribed
    if (std::find(dataEventListeners.begin(), dataEventListeners.end(),
                  listener) == dataEventListeners.end()) {
      dataEventListeners.push_back(listener);
    }
  }

  inline void publishData(const DataEvent &event) const {
    for (const auto &listener : dataEventListeners) {
      listener->applyData(event);
    }
  }
};

} // namespace Observer