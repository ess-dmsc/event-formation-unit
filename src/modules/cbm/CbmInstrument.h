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

#include <common/geometry/DetectorGeometry.h>
#include <common/memory/HashMap2D.h>
#include <common/Statistics.h>
#include <modules/cbm/Counters.h>
#include <modules/cbm/geometry/Geometry.h>
#include <modules/cbm/geometry/Config.h>
#include <modules/cbm/readout/Parser.h>
#include <modules/cbm/SchemaDetails.h>
#include <unordered_map>

namespace cbm {

class CbmInstrument {
public:
  using DA00Serializer_t = SchemaDetails::DA00Serializer_t;
  using EV44Serializer_t = SchemaDetails::EV44Serializer_t;
  using GeometryMap_t = std::unordered_map<uint16_t, std::unique_ptr<Geometry>>;

  /// \brief Constructor for the CbmInstrument class.
  /// \details The constructor initializes the instrument and sets the
  /// references to the counters, configuration data, and serializers.
  ///
  /// \param Stats Reference to Statistics object for counter registration.
  /// \param counters Reference to the counters for the CBM instrument.
  /// \param Config Reference to the configuration data for the CBM instrument.
  /// \param SchemaDetailMap Reference to the HashMap2D of Serializers
  /// \param essHeaderParser Header parser
  CbmInstrument(Statistics &Stats, Counters &counters, Config &Config,
                const HashMap2D<SchemaDetails> &SchemaDetailMap,
                ESSReadout::Parser &essHeaderParser);

  /// \brief Process the beam monitor readouts.
  void processMonitorReadouts(void);

  /// \brief Get read access to geometry object to access internal stats
  /// counters.
  ///
  /// \param FENId FEN identifier (8-bit)
  /// \param ChannelId Channel identifier (8-bit)
  /// \return  Base geometry object.
  const cbm::Geometry *GetGeometry(uint8_t FENId, uint8_t ChannelId) {
    uint16_t key = calcGeometryKey(FENId, ChannelId);
    std::unique_ptr<cbm::Geometry> &baseGeometry = Geometries[key];
    return baseGeometry.get();
  }

  /// \brief Parser for CBM readout data.
  Parser CbmReadoutParser;

private:

  /// \brief Reference to the counters for the CBM instrument.
  struct Counters &counters;

  /// \brief Reference to the configuration data for the CBM instrument.
  Config &Conf;

  /// \brief References for the serializers of the supported types.
  const HashMap2D<SchemaDetails> &SchemaMap;

  /// \brief Parser for the ESS Readout header.
  ESSReadout::Parser &ESSHeaderParser;

  /// \brief Geometries map for all beam monitor types.
  /// Key is created with FEN id as the 8 most significant bits and channel id
  /// as 8 least significant bits. Stores DetectorGeometry base class pointers
  /// for polymorphic access to validation and pixel calculation.
  GeometryMap_t Geometries{};
  //std::unordered_map<uint16_t, std::unique_ptr<geometry::DetectorGeometry<Parser::CbmReadout>>> Geometries{};

  /// \brief Calculate geometry map key from FEN and Channel IDs
  /// \param FENId FEN identifier (8-bit)
  /// \param ChannelId Channel identifier (8-bit)
  /// \return 16-bit key with FEN in upper byte and Channel in lower byte
  inline uint16_t calcGeometryKey(uint8_t FENId, uint8_t ChannelId) const {
    return (static_cast<uint16_t>(FENId) << 8) | ChannelId;
  }
};

} // namespace cbm
