


#include <common/dataflow/DataObserverTemplate.h>
#include <readout/DataEventTypes.h>
#include <vector>
#include <memory>

#pragma once

namespace Timepix3 {

using namespace Observer;

class TimingEventHandler : public DataEventListener<TDCDataEvent>, public DataEventListener<EVRDataEvent>{
    private:
        std::unique_ptr<TDCDataEvent> lastTdcFrame;
        uint64_t nextTDCTimeStamp;
        uint32_t frequency;

    public:
        void applyData(const TDCDataEvent& newData) override;
        void applyData(const EVRDataEvent& newData) override;

        uint64_t getLastTDCTimestamp();

        uint32_t getTDCFrequency() const;
};
}