// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for reading Loki readout data from HDF5 files
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <common/debug/Hexdump.h>
#include <common/readout/ess/Parser.h>
#include <loki/generators/ReaderReadouts.h>
#include <loki/readout/DataParser.h>
#include <loki/readout/Readout.h>
#include <iostream>

namespace Loki {

ReaderReadouts::ReaderReadouts(std::string filename) {
  file = ReadoutFile::open(filename);
  total_ = file->count();
  ReadoutSize = sizeof(Readout);
  ChunkSize = ReadoutFile::ChunkSize;
  current_ = 0;
  //printf("total %lu, RDSize %lu, ChunkSize %lu\n",
  //        total_, ReadoutSize, ChunkSize);
}

size_t ReaderReadouts::read(char *buf) {
  size_t size = ReadoutFile::ChunkSize;
  if ((current_ + ReadoutFile::ChunkSize) > total_) {
    size = total_ - current_;
  }

  size_t TotalLength{0};
  const size_t LokiReadoutSize = sizeof(struct DataParser::LokiReadout);
  const size_t ESSHeaderSize = sizeof(struct ESSReadout::Parser::PacketHeaderV0);

  if (size > 0) {
    try {
      file->readAt(current_, size);
      Readout * Loki = (Readout *)(file->Data.data());
      auto * PHP = (ESSReadout::Parser::PacketHeaderV0 *)buf;

      PHP->Padding0 = 0x00;
      PHP->Version = 0x00;
      PHP->CookieAndType =
            (ESSReadout::Parser::DetectorType::Loki4Amp << 24) + 0x00535345;
      PHP->TotalLength =  ESSHeaderSize + size * (LokiReadoutSize);
      PHP->OutputQueue = 0x00;
      PHP->TimeSource = 0x00; /// \todo hardcoded
      PHP->PulseHigh = Loki[0].PulseTimeHigh;
      PHP->PulseLow = Loki[0].PulseTimeLow;
      PHP->PrevPulseHigh = Loki[0].PrevPulseTimeHigh;
      PHP->PrevPulseLow = Loki[0].PrevPulseTimeLow;
      PHP->SeqNum = SeqNum++;

      auto * LRDP = (struct DataParser::LokiReadout *)(buf
                    + sizeof(struct ESSReadout::Parser::PacketHeaderV0));

      if (Loki[0].PulseTimeHigh != Loki[size - 1].PulseTimeHigh) {
        printf("TimeHigh inconsistent with packet\n");
      }
      if (Loki[0].PulseTimeLow != Loki[size - 1].PulseTimeLow) {
        printf("TimeLow inconsistent with packet\n");
      }

      for (unsigned int i = 0; i < size; i++) {
        LRDP[i].RingId = Loki[i].RingId;
        LRDP[i].FENId = Loki[i].FENId;
        LRDP[i].DataLength = 24;
        LRDP[i].TimeHigh = Loki[i].EventTimeHigh;
        LRDP[i].TimeLow = Loki[i].EventTimeLow;
        LRDP[i].unused = 0x00;
        LRDP[i].TubeId = Loki[i].TubeId;
        LRDP[i].DataSeqNum = 0x0000;
        LRDP[i].AmpA = Loki[i].AmpA;
        LRDP[i].AmpB = Loki[i].AmpB;
        LRDP[i].AmpC = Loki[i].AmpC;
        LRDP[i].AmpD = Loki[i].AmpD;
      }
      TotalLength = PHP->TotalLength;

    } catch (std::exception &e) {
      std::cout << "<ReaderReadouts> failed to read slab ("
                << current_ << ", " << (current_ + size) << ")"
                << " max=" << total_ << "\n"
                << hdf5::error::print_nested(e, 1) << std::endl;
    }
  }

  current_ += size;

  hexDump(buf, TotalLength);

  printf("Packet of length %zu\n", TotalLength);
  return TotalLength;
}

}
// GCOVR_EXCL_STOP
