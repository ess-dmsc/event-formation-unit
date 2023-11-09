// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Listener interface for data events
//===----------------------------------------------------------------------===//

#pragma once

#include <readout/DataEventListener.h>
#include <cstdint>
#include <vector>

namespace Timepix3 {

template<typename DataEvent>
class DataEventManager {
    private:
        std::vector<DataEventListener<DataEvent>* > dataEventListeners;

    public:
        DataEventManager<DataEvent>() {};
        void addListener(DataEventListener<DataEvent> *listener);
        void notifyListeners(const DataEvent &event);
};

}