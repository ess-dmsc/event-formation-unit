// Copyright (C) 2019 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Parser for ESS readout of Caen Modules
//===----------------------------------------------------------------------===//

#pragma once

#include <common/readout/ess/Parser.h>
#include <vector>

namespace Caen {

class DataParser {
public:
  const unsigned int MaxFiberId{23};
  const unsigned int MaxFENId{23};
  const unsigned int MaxReadoutsInPacket{500};

  /// \todo Multiple issues regarding amplitudes
  // This struct is only superficially correct
  /// Regarding signed vs unsigned this is correct only for LOKI
  /// where the four amplitudes are actually signed integers.
  /// For BIFROST and TBL3HE (and presumably MIRACLES, VESPA, CSPEC too)
  /// the amplitudes are unsigned.
  // Currently (for BIFROST) thuis is taken care of in the code. But we
  // should probably fix this at some point
  ///
  /// For the identification of the amplitudes this is NOT correct for
  /// LOKI as the amplitudes A and B are swapped and C and D are swapped.
  /// this is 'corrected' in the charge division formulae for LOKI.
  /// For BIFROST (and the others) these are correct.
  struct CaenReadout {
    uint8_t FiberId;
    uint8_t FENId;
    uint16_t DataLength;
    uint32_t TimeHigh;
    uint32_t TimeLow;
    uint8_t FlagsOM;
    uint8_t Group;
    uint16_t Unused;
    int16_t AmpA;
    int16_t AmpB;
    int16_t AmpC;
    int16_t AmpD;
  } __attribute__((__packed__));

  struct Stats {
    int64_t DataHeaders{0};
    int64_t Readouts{0};
    int64_t ReadoutsMaxADC{0};
    int64_t RingFenErrors{0};
    int64_t DataLenMismatch{0};
    int64_t DataLenInvalid{0};
    int64_t DataHeaderSizeErrors{0};
  };

  static_assert(sizeof(CaenReadout) == 24, "Caen readout header length error");

  DataParser() { Result.reserve(MaxReadoutsInPacket); };
  ~DataParser(){};

  //
  int parse(const char *buffer, unsigned int size);

  // To be iterated over in processing thread
  std::vector<struct CaenReadout> Result;

  struct Stats Stats;
};
} // namespace Caen
