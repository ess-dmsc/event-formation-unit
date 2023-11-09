


#include <common/dataflow/DataObserver.h>
#include <readout/DataEventTypes.h>
#include <vector>
#include <memory>

#pragma once

namespace Timepix3 {

using namespace Observer;

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