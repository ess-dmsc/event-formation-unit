// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Listener interface for data events
//===----------------------------------------------------------------------===//

#pragma once

#include "DataEventTypes.h"
#include <cstdint>
#include <locale>
#include <vector>
#include <memory>

namespace Timepix3 {

template<typename DataEvent>
class DataEventListener {
    public:
        virtual void notify(const DataEvent& event) = 0;
};

class TimingEventHandler : public DataEventListener<TDCData> {
    private:
        std::unique_ptr<TDCData> lastTdcFrame;
        uint64_t nextTDCTimeStamp;
        uint32_t frequency;

    public:
        void notify(const TDCData& newData) override;

        uint64_t getLastTDCTimestamp();

        uint32_t getTDCFrequency() const;
};

}