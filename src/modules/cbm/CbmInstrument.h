// Copyright (C) 2022-2024 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Cbm is responsible for readout validation and
/// common beam monitor 'event formation'
/// Its functions are called from the main prcessing loop in CbmBase
//===----------------------------------------------------------------------===//

#pragma once

#include <common/detector/BaseSettings.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/serializer/DA00HistogramSerializer.h>
#include <common/memory/HashMap2D.h>
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
                    &HistogramSerializerPtrs);

  /// \brief Process the beam monitor readouts.
  void processMonitorReadouts(void);

public:
  /// \brief Reference to the counters for the CBM instrument.
  struct Counters &counters;

  /// \brief Reference to the configuration data for the CBM instrument.
  Config &Conf;

  /// \brief References for the serializers of the supported types.
  const HashMap2D<EV44Serializer> &Ev44SerializerMap;
  const HashMap2D<fbserializer::HistogramSerializer<int32_t>>
      &HistogramSerializerMap;

  /// \brief Parser for the ESS Readout header.
  ESSReadout::Parser ESSHeaderParser;

  /// \brief Parser for CBM readout data.
  Parser CbmReadoutParser;
};

} // namespace cbm
