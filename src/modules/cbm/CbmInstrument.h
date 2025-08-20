// Copyright (C) 2022 - 2025 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Cbm is responsible for readout validation and
/// common beam monitor 'event formation'
/// Its functions are called from the main processing loop in CbmBase
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/Parser.h>
#include <common/detector/BaseSettings.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/serializer/DA00HistogramSerializer.h>
#include <common/memory/HashMap2D.h>
#include <logical_geometry/ESSGeometry.h>
#include <modules/cbm/Counters.h>
#include <modules/cbm/geometry/Config.h>
#include <modules/cbm/geometry/Parser.h>

namespace cbm {

class CbmInstrument {
public:
  /// \brief Constructor for the CbmInstrument class.
  /// \details The constructor initializes the instrument and sets the
  /// references to the counters, configuration data, and serializers.
  ///
  /// \param counters Reference to the counters for the CBM instrument.
  /// \param Config Reference to the configuration data for the CBM instrument.
  /// \param Ev44SerializerPtrs Reference to the HashMap2D of EV44Serializer
  /// pointers. \param HistogramSerializerPtrs Reference to the HashMap2D of
  /// HistogramSerializer pointers.
  CbmInstrument(Counters &counters, Config &Config,
                const HashMap2D<EV44Serializer> &Ev44SerializerPtrs,
                const HashMap2D<fbserializer::HistogramSerializer<int32_t>>
                    &HistogramSerializerPtrs,
                ESSReadout::Parser &essHeaderParser);

  /// \brief Process the beam monitor readouts.
  void processMonitorReadouts(void);

  /// \brief Parser for CBM readout data.
  Parser CbmReadoutParser;

private:
  /// \brief Reference to the counters for the CBM instrument.
  struct Counters &counters;

  /// \brief Reference to the configuration data for the CBM instrument.
  Config &Conf;

  /// \brief References for the serializers of the supported types.
  const HashMap2D<EV44Serializer> &Ev44SerializerMap;
  const HashMap2D<fbserializer::HistogramSerializer<int32_t>>
      &HistogramSerializerMap;

  /// \brief Parser for the ESS Readout header.
  ESSReadout::Parser &ESSHeaderParser;
  /// \brief Geometries is a map with the individual ESS geometry used for each 
  /// EVENT_2D beam monitor in the topology file. 
  /// Key is created with FEN id as the 8 most significant bits and channel id
  /// as 8 least significant bits
  std::unordered_map<uint16_t, std::unique_ptr<ESSGeometry>> Geometries{};
};

} // namespace cbm
