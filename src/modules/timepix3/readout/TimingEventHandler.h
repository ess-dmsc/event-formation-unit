// Copyright (C) 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation for TDC and EVR timing event observers. Responsible
//         to follow up different timing events and syncronize the two timing 
//         (ESS time and camera time) domains. Also this class provides timing
//         information for other timing interested objects.
//===----------------------------------------------------------------------===//

#include <dataflow/DataObserverTemplate.h>
#include <readout/DataEventTypes.h>
#include <vector>
#include <memory>

#pragma once

namespace Timepix3 {

using namespace Observer;

class TimingEventHandler : public DataEventObserver<TDCDataEvent> {
    private:
        std::shared_ptr<TDCDataEvent> lastTdcFrame;
        uint64_t nextTDCTimeStamp;
        uint32_t frequency;

    public:
        void applyData(const TDCDataEvent& newData) override;

        uint64_t getLastTDCTimestamp();

        uint32_t getTDCFrequency() const;

        const std::shared_ptr<TDCDataEvent> getLastTdcEvent() const;
};
}