// Copyright (C) 2022 - 2026 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Cbm is responsible for readout validation and
/// common beam monitor 'event formation'
/// Its functions are called from the main processing loop in CbmBase
//===----------------------------------------------------------------------===//

#pragma once

#include <common/Statistics.h>
#include <common/memory/HashMap2D.h>
#include <modules/cbm/Counters.h>
#include <modules/cbm/SchemaDetails.h>
#include <modules/cbm/geometry/Config.h>
#include <modules/cbm/geometry/Geometry.h>
#include <modules/cbm/readout/Parser.h>

namespace cbm {

class CbmInstrument {
public:
  using DA00Serializer_t = SchemaDetails::DA00Serializer_t;
  using EV44Serializer_t = SchemaDetails::EV44Serializer_t;

  /// \brief Constructor for the CbmInstrument class.
  /// \details The constructor initializes the instrument and sets the
  /// references to the counters, configuration data, and serializers.
  ///
  /// \param Stats Reference to Statistics object for counter registration.
  /// \param counters Reference to the counters for the CBM instrument.
  /// \param Config Reference to the configuration data for the CBM instrument.
  /// \param SchemaDetailMap Reference to the HashMap2D of Serializers
  /// \param cbmReadoutParser CBM readout parser
  /// \param essHeaderParser Header parser
  CbmInstrument(Statistics &Stats, Counters &counters, Config &Config,
                Parser &cbmReadoutParser,
                const HashMap2D<SchemaDetails> &SchemaDetailMap,
                ESSReadout::Parser &essHeaderParser);

  /// \brief Process the beam monitor readouts.
  void processMonitorReadouts(void);

  /// \brief Get read access to the single geometry object
  ///
  /// \return Reference to Geometry object
  const Geometry &GetGeometry() const {
    return CbmGeometry;
  }

  /// \brief Reference to parser for CBM readout data.
  Parser &CbmReadoutParser;

private:
  /// \brief Reference to the counters for the CBM instrument.
  struct Counters &counters;

  /// \brief Reference to the configuration data for the CBM instrument.
  Config &Conf;

  /// \brief References for the serializers of the supported types.
  const HashMap2D<SchemaDetails> &SchemaMap;

  /// \brief Parser for the ESS Readout header.
  ESSReadout::Parser &ESSHeaderParser;

  /// \brief Single geometry object for all beam monitor types
  Geometry CbmGeometry;
};

} // namespace cbm
