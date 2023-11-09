// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Listener interface for data events
//===----------------------------------------------------------------------===//

#pragma once

#include "DataEventTypes.h"
#include <locale>
#include <vector>
#include <memory>

namespace Timepix3 {

template<typename DataEvent>
class DataEventListener {
    public:
        virtual void notify(const DataEvent& event) = 0;
};

class TimingEventHandler : public DataEventListener<TDCDataEvent> {
    private:
        std::unique_ptr<TDCDataEvent> lastTdcFrame;

    public:
        void notify(const TDCDataEvent& newData) override {
            lastTdcFrame = std::make_unique<TDCDataEvent>(newData);
        }
};

}