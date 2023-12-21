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

#pragma once

#include "Counters.h"
#include "common/kafka/EV44Serializer.h"
#include "common/reduction/clustering/Hierarchical2DClusterer.h"
#include "dataflow/DataObserverTemplate.h"
#include "geometry/Timepix3Geometry.h"
#include "readout/PixelDataEvent.h"

namespace Timepix3 {

using namespace Observer;
using namespace std;
using namespace chrono;

class PixelEventHandler : public DataEventObserver<PixelDataEvent>,
                          public DataEventObserver<EpochESSPulseTime> {

private:
  Counters &statCounters;
  shared_ptr<Timepix3Geometry> geometry;
  Hierarchical2DClusterer &clusterer;
  EV44Serializer &serializer;
  unique_ptr<EpochESSPulseTime> lastEpochESSPulseTime = nullptr;

  Hit2DVector allHitsVector;

  void generateEvents();
  uint64_t calculateGlobalTime(const uint16_t &toa, const uint8_t &fToA,
                               const uint32_t &spidrTime);

public:
  PixelEventHandler(Counters &statCounters,
                    shared_ptr<Timepix3Geometry> geometry,
                    Hierarchical2DClusterer &clusterer,
                    EV44Serializer &serializer)
      : statCounters(statCounters), geometry(geometry), clusterer(clusterer),
        serializer(serializer), allHitsVector(Hit2DVector()){};

  virtual ~PixelEventHandler(){};

  void applyData(const PixelDataEvent &pixelDataEvent) override;
  void applyData(const EpochESSPulseTime &epochEssPulseTime) override;

  void pushDataToKafka();
};

} // namespace Timepix3