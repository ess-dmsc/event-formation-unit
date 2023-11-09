// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Listener interface for data events
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdint>
#include <locale>
#include <vector>
#include <memory>

namespace Observer {

template <typename DataEvent> class DataEventListener {
public:
  virtual void applyData(const DataEvent &event) = 0;
};

template<typename DataEvent>
class DataEventPublisher {
    private:
        std::vector<DataEventListener<DataEvent>* > dataEventListeners;

    public:
        DataEventPublisher<DataEvent>() {};
        void subscribe(DataEventListener<DataEvent> *listener) {
    dataEventListeners.push_back(listener);
}

        void publishData(const DataEvent &event) const {
            for(const auto& listener : dataEventListeners) {
                listener->applyData(event);
                }
}
};

}

namespace Timepix3 {

using namespace Observer;

}