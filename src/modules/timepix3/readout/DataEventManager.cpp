// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Listener interface for data events
//===----------------------------------------------------------------------===//

#include "readout/DataEventManager.h"
#include <vector>

namespace Timepix3 {

template<typename DataEvent>
void DataEventManager<DataEvent>::addListener(DataEventListener<DataEvent> *listener) {
    dataEventListeners.push_back(listener);
}

template<typename DataEvent>
void DataEventManager<DataEvent>::notifyListeners(const DataEvent &event) {
   for(const auto& listener : dataEventListeners) {
    listener->notify(event);
   }
}

}