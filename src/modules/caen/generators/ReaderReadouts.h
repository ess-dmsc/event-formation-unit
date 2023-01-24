// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for reading Loki readout data from HDF5 files
///
/// Created after the STFC/ISIS test March 2022 to compensate for a few
/// missing pcap dumps. Replay times are identical to the original timestamps
/// so any subsequent file writing must write files from the past.
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <common/readout/ess/Parser.h>
#include <modules/caen/readout/DataParser.h>
#include <modules/caen/readout/Readout.h>

namespace Caen {

class ReaderReadouts {
public:
  /// \brief open H5 file for later reading, save ChunkSize
  /// also sets a few internal counters.
  ReaderReadouts(std::string filename);

  /// \brief read from H5 file and format an ESS Readout data
  /// The read function keeps track of changing pulse times and
  /// adjust the fileoffset for reading accordingly. The file is
  /// always read in chunks of Chunksize. For low data rates this
  /// is inefficient but we don't care ;-)
  size_t read(char *buf);

  /// ReadoutSize refers to <loki/readout/Readout.h>
  size_t getReadoutSize() const { return ReadoutSize; }

  /// ChunkSize is a HDF5 specific parameter.
  size_t getChunkSize() const { return ChunkSize; }

private:
  std::shared_ptr<ReadoutFile> file;

  size_t total_{0};
  size_t current_{0};
  size_t ReadoutSize{0};
  size_t ChunkSize{0};
  uint32_t SeqNum{0};
  const size_t LokiReadoutSize = sizeof(struct DataParser::CaenReadout);
  const size_t ESSHeaderSize =
      sizeof(struct ESSReadout::Parser::PacketHeaderV0);

  // Need to keep track of when this changes so we can start a
  // new packet.
  uint32_t CurPulseTimeHigh{0};
  uint32_t CurPulseTimeLow{0};
};

} // namespace Caen

// GCOVR_EXCL_STOP
