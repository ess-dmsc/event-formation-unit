// Copyright (C) 2024 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Implementation for Timepix3 pixel global time calculation and
///        clustering.
//===----------------------------------------------------------------------===//

#pragma once

#include <modules/timepix3/Counters.h>
#include <common/kafka/EV44Serializer.h>
#include <common/reduction/Hit2DVector.h>
#include <common/reduction/clustering/Hierarchical2DClusterer.h>
#include <common/utils/EfuUtils.h>
#include <dataflow/DataObserverTemplate.h>
#include <geometry/Timepix3Geometry.h>
#include <dto/TimepixDataTypes.h>
#include <future>
#include <memory>
#include <vector>

namespace Timepix3 {

class PixelEventHandler
    : public Observer::DataEventObserver<timepixReadout::PixelReadout>,
      public Observer::DataEventObserver<timepixDTO::ESSGlobalTimeStamp> {

private:
  Counters &statCounters;
  std::shared_ptr<Timepix3Geometry> geometry;
  EV44Serializer &serializer;
  std::unique_ptr<timepixDTO::ESSGlobalTimeStamp> lastEpochESSPulseTime =
      nullptr;

  std::vector<std::unique_ptr<Hierarchical2DClusterer>> clusterers;
  std::vector<Hit2DVector> windows;

  void publishEvents(Cluster2DContainer &clusters);
  void clusterHits(Hierarchical2DClusterer &, Hit2DVector &hitsVector);

  uint64_t calculateGlobalTime(const uint16_t &toa, const uint8_t &fToA,
                               const uint32_t &spidrTime);

public:
  PixelEventHandler(Counters &, std::shared_ptr<Timepix3Geometry>,
                    EV44Serializer &);

  virtual ~PixelEventHandler(){};

  void applyData(const timepixReadout::PixelReadout &pixelDataEvent) override;
  void
  applyData(const timepixDTO::ESSGlobalTimeStamp &epochEssPulseTime) override;

  void pushDataToKafka();
};

} // namespace Timepix3