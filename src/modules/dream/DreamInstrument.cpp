// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating DREAM processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <common/Trace.h>
#include <common/TimeString.h>
#include <dream/DreamInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Jalousie {

  DreamInstrument::DreamInstrument(struct Counters & counters,
      DreamSettings &moduleSettings)
        : counters(counters)
        , ModuleSettings(moduleSettings) { }


  uint32_t DreamInstrument::calcPixel() {
      return 42;
  }

  void DreamInstrument::processReadouts() {
    // Dont fake pulse time, but could do something like
    // PulseTime = 1000000000LU * (uint64_t)time(NULL); // ns since 1970
    uint64_t PulseTime;

    auto PacketHeader = ESSReadoutParser.Packet.HeaderPtr;
    PulseTime = Time.setReference(PacketHeader->PulseHigh,PacketHeader->PulseLow);
    XTRACE(DATA, DEB, "PulseTime (%u,%u) %" PRIu64 "", PacketHeader->PulseHigh,
      PacketHeader->PulseLow, PulseTime);
    Serializer->pulseTime(PulseTime);

    /// Traverse readouts, calculate pixels
    for (auto & Section : DreamParser.Result) {
      XTRACE(DATA, DEB, "Ring %u, FEN %u", Section.RingId, Section.FENId);

      // if (Section.RingId >= LokiConfiguration.Panels.size()) {
      //   XTRACE(DATA, WAR, "RINGId %d is incompatible with configuration", Section.RingId);
      //   counters.MappingErrors++;
      //   continue;
      // }
    //
    //
      if (Section.FENId == 0) {
        XTRACE(DATA, WAR, "FENId == 0");
        counters.MappingErrors++;
        continue;
      }

      for (auto & Data : Section.Data) {
        auto TimeOfFlight =  Time.getTOF(0, Data.Tof); // TOF in ns

        XTRACE(DATA, DEB, "  Data: time (0, %u), mod %u, sumo %u, strip %u, wire %u, seg %u, ctr %u",
          Data.Tof, Data.Module, Data.Sumo, Data.Strip, Data.Wire, Data.Segment, Data.Counter);

        // Calculate pixelid and apply calibration
        uint32_t PixelId = calcPixel();


        if (PixelId == 0) {
          counters.GeometryErrors++;
        } else {
          counters.TxBytes += Serializer->addEvent(TimeOfFlight, PixelId);
          counters.Events++;
        }
      }
    }
  }


  //
  DreamInstrument::~DreamInstrument() { }

} // namespace
