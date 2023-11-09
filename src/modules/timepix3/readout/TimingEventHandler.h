


#include <common/dataflow/DataObserverTemplate.h>
#include <readout/DataEventTypes.h>
#include <vector>
#include <memory>

#pragma once

namespace Timepix3 {

using namespace Observer;

class TimingEventHandler : public DataEventListener<TDCDataEvent> {
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