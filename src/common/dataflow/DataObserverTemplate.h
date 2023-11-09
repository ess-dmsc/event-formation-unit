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

template <typename DataEvent> class DataEventObserver {
public:
  virtual void applyData(const DataEvent &event) = 0;
};

template<typename DataEvent>
class DataEventObservable {
    private:
        std::vector<DataEventObserver<DataEvent>* > dataEventListeners;

    public:
        DataEventObservable<DataEvent>() {};
        void subscribe(DataEventObserver<DataEvent> *listener) {
            dataEventListeners.push_back(listener);
        }

        void publishData(const DataEvent &event) const {
            for(const auto &listener : dataEventListeners) {
                listener->applyData(event);
            }
        }
};

}

namespace Timepix3 {

using namespace Observer;

}