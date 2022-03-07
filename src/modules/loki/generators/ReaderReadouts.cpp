// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for generating UDP from Loki readout HDF5 files
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <loki/generators/ReaderReadouts.h>
#include <loki/readout/Readout.h>
#include <iostream>

namespace Loki {

ReaderReadouts::ReaderReadouts(std::string filename) {
  file = ReadoutFile::open(filename);
  total_ = file->count();
  ReadoutSize = sizeof(Readout);
  ChunkSize = ReadoutFile::ChunkSize;
}


size_t ReaderReadouts::read(char *buf) {
  size_t size = ChunkSize;
  if ((current_ + ChunkSize) > total_) {
    size = total_ - current_;
  }

  size_t TotalLength{0};
  unsigned int CurDataItems{0};

  if (size > 0) {
    try {
      file->readAt(current_, size);
      Readout * LokiData = (Readout *)(file->Data.data());
      auto * PacketHdr = (ESSReadout::Parser::PacketHeaderV0 *)buf;

      PacketHdr->Padding0 = 0x00;
      PacketHdr->Version = 0x00;
      PacketHdr->CookieAndType =
            (ESSReadout::Parser::DetectorType::Loki4Amp << 24) + 0x00535345;
      PacketHdr->TotalLength = ESSHeaderSize;
      PacketHdr->OutputQueue = 0x00;
      PacketHdr->TimeSource = 0x00; /// \todo hardcoded
      PacketHdr->PulseHigh = LokiData[0].PulseTimeHigh;
      PacketHdr->PulseLow = LokiData[0].PulseTimeLow;
      PacketHdr->PrevPulseHigh = LokiData[0].PrevPulseTimeHigh;
      PacketHdr->PrevPulseLow = LokiData[0].PrevPulseTimeLow;
      PacketHdr->SeqNum = SeqNum++;

      auto * LRDP = (struct DataParser::LokiReadout *)(buf
                    + sizeof(struct ESSReadout::Parser::PacketHeaderV0));

      CurPulseTimeHigh = LokiData[0].PulseTimeHigh;
      CurPulseTimeLow = LokiData[0].PulseTimeLow;

      for (unsigned int i = 0; i < size; i++) {
        if ((CurPulseTimeHigh != LokiData[i].PulseTimeHigh) or
            (CurPulseTimeLow  != LokiData[i].PulseTimeLow )   ) {
          break;
        }
        LRDP[i].RingId = LokiData[i].RingId;
        LRDP[i].FENId = LokiData[i].FENId;
        LRDP[i].DataLength = 24;
        LRDP[i].TimeHigh = LokiData[i].EventTimeHigh;
        LRDP[i].TimeLow = LokiData[i].EventTimeLow;
        LRDP[i].unused = 0x00;
        LRDP[i].TubeId = LokiData[i].TubeId;
        LRDP[i].DataSeqNum = 0x0000;
        LRDP[i].AmpA = LokiData[i].AmpA;
        LRDP[i].AmpB = LokiData[i].AmpB;
        LRDP[i].AmpC = LokiData[i].AmpC;
        LRDP[i].AmpD = LokiData[i].AmpD;
        CurDataItems++;
        PacketHdr->TotalLength += LokiReadoutSize;
        TotalLength = PacketHdr->TotalLength;
      }

    } catch (std::exception &e) {
      std::cout << "<ReaderReadouts> failed to read slab ("
                << current_ << ", " << (current_ + size) << ")"
                << " max=" << total_ << "\n"
                << hdf5::error::print_nested(e, 1) << std::endl;
    }
  }

  current_ += CurDataItems;
  return TotalLength;
}

}
// GCOVR_EXCL_STOP
