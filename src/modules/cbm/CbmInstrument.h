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

#include <common/readout/ess/Parser.h>
#include <common/detector/BaseSettings.h>
#include <common/geometry/DetectorGeometry.h>
#include <common/kafka/EV44Serializer.h>
#include <common/kafka/serializer/DA00HistogramSerializer.h>
#include <common/memory/HashMap2D.h>
#include <common/Statistics.h>
#include <modules/cbm/Counters.h>
#include <modules/cbm/geometry/Geometry2D.h>
#include <modules/cbm/geometry/Geometry0D.h>
#include <modules/cbm/geometry/Config.h>
#include <modules/cbm/readout/Parser.h>

namespace cbm {

class CbmInstrument {
public:
  using HistogramSerializer_t = fbserializer::HistogramSerializer<int32_t, int32_t, uint64_t>;

  /// \brief Constructor for the CbmInstrument class.
  /// \details The constructor initializes the instrument and sets the
  /// references to the counters, configuration data, and serializers.
  ///
  /// \param Stats Reference to Statistics object for counter registration.
  /// \param counters Reference to the counters for the CBM instrument.
  /// \param Config Reference to the configuration data for the CBM instrument.
  /// \param Ev44SerializerPtrs Reference to the HashMap2D of EV44Serializer
  /// pointers. \param HistogramSerializerPtrs Reference to the HashMap2D of
  /// HistogramSerializer pointers.
  CbmInstrument(Statistics &Stats, Counters &counters, Config &Config,
                const HashMap2D<EV44Serializer> &Ev44SerializerPtrs,
                const HashMap2D<HistogramSerializer_t> &HistogramSerializerPtrs,
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
  const HashMap2D<HistogramSerializer_t> &HistogramSerializerMap;

  /// \brief Parser for the ESS Readout header.
  ESSReadout::Parser &ESSHeaderParser;

  /// \brief Geometries map for all beam monitor types.
  /// Key is created with FEN id as the 8 most significant bits and channel id
  /// as 8 least significant bits. Stores DetectorGeometry base class pointers
  /// for polymorphic access to validation and pixel calculation.
  std::unordered_map<uint16_t, std::unique_ptr<geometry::DetectorGeometry<Parser::CbmReadout>>> Geometries{};

  /// \brief Calculate geometry map key from FEN and Channel IDs
  /// \param FENId FEN identifier (8-bit)
  /// \param ChannelId Channel identifier (8-bit)
  /// \return 16-bit key with FEN in upper byte and Channel in lower byte
  inline uint16_t calcGeometryKey(uint8_t FENId, uint8_t ChannelId) const {
    return (static_cast<uint16_t>(FENId) << 8) | ChannelId;
  }
};

} // namespace cbm
