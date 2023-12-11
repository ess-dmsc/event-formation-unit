// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Simple implementation for the observer pattern with template
/// observable
//===----------------------------------------------------------------------===//

#pragma once

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

  void subscribe(DataEventObserver<DataEvent> *listener) {
    dataEventListeners.push_back(listener);
  }

  void publishData(const DataEvent &event) const {
    for (const auto &listener : dataEventListeners) {
      listener->applyData(event);
    }
  }
};

} // namespace Observer

namespace Timepix3 {

using namespace Observer;

}