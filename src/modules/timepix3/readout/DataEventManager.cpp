// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Listener interface for data events
//===----------------------------------------------------------------------===//

#include "DataEventManager.h"
#include <cstdint>


namespace Timepix3 {

// Explicit instantiation for templates in use
template class DataEventManager<TDCDataEvent>;

template<typename DataEvent>
void DataEventManager<DataEvent>::addListener(DataEventListener<DataEvent> *listener) {
    dataEventListeners.push_back(listener);
}

template<typename DataEvent>
void DataEventManager<DataEvent>::notifyListeners(const DataEvent &event) const {
            for(const auto& listener : dataEventListeners) {
                listener->notify(event);
                }
}

}