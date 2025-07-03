// Copyright (C) 2019 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief CAEN application counters (for Grafana and low level debug)
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <common/kafka/Producer.h>
#include <cstdint>
#include <modules/caen/geometry/CDCalibration.h>
#include <modules/caen/geometry/Geometry.h>
#include <modules/caen/readout/DataParser.h>

namespace Caen {

struct CaenCounters {
  // Processing Counters - accessed in processing thread

  // System counters
  int64_t FifoSeqErrors{0};
  int64_t ProcessingIdle{0};

  // ESSReadout parser
  int64_t ErrorESSHeaders{0};
  
  // Caen DataParser
  struct DataParser::Stats Parser;
  
  // Logical and Digital geometry incl. Calibration
  struct CDCalibration::Stats Calibration;
  struct Geometry::Stats Geom;
  
  // Events
  int64_t Events{0};
  int64_t PixelErrors{0};
  int64_t TimeError{0};
  int64_t EventsUdder{0};
  int64_t TxRawReadoutPackets{0};
  int64_t SerializerErrors{0};

  // Identification of the cause of produce calls
  int64_t ProduceCauseTimeout{0};
  int64_t ProduceCausePulseChange{0};
  int64_t ProduceCauseMaxEventsReached{0};

} __attribute__((aligned(64)));
} // namespace Caen
