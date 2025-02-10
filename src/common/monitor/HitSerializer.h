// Copyright (C) 2016 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief flatbuffer serialization
///
//===----------------------------------------------------------------------===//

#pragma once

#include "mo01_nmx_generated.h"

#include <common/kafka/Producer.h>
#include <functional>

class HitSerializer {
public:
  /// \brief Create the HitSerializer
  /// \param maxentries the number of readout tuples to buffer before sending to
  /// Kafka
  HitSerializer(size_t maxentries, const std::string &source_name);

  void set_callback(ProducerCallback cb);

  /// \todo use generic Hit struct
  /// \brief function to add a readout tuple to an array for later publishing to
  /// Kafka \param plane arbitrary data multiplexer (x, y, module, rack, ...)
  /// \param channel datasource identifier (strip, wire, grid, ...)
  /// \param time timestamp
  /// \param adc digitizer adc value (charge, voltage, ...)
  size_t addEntry(uint16_t plane, uint16_t channel, uint32_t time,
                  uint16_t adc);

  /// \brief publish data to Kafka broker and clear internal counters
  size_t produce();

  /// \brief return the number of queues samples
  size_t getNumEntries() { return entries; };

private:
  ProducerCallback producer_callback;

  size_t maxlen{0}; ///< maximum number of entries in array
  flatbuffers::FlatBufferBuilder builder; ///< google flatbuffer builder

  std::string SourceName;

  // Will be used to create MONHit
  size_t entries{0};              ///< current number of queues entries
  std::vector<uint16_t> planes;   ///< described above
  std::vector<uint32_t> times;    ///< described above
  std::vector<uint16_t> channels; ///< described above
  std::vector<uint16_t> adcs;     ///< described above
};
