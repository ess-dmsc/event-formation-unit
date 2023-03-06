// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Timepix3 processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <timepix3/Timepix3Instrument.h>
#include <common/debug/Log.h>
#include <common/debug/Trace.h>
#include <common/time/TimeString.h>
#include <fmt/format.h>

#undef TRC_LEVEL
#define TRC_LEVEL TRC_L_DEB

namespace Timepix3 {

/// \brief load configuration and calibration files, throw exceptions
/// if these have errors or are inconsistent
///
/// throws if number of pixels do not match, and if the (invalid) pixel
/// value 0 is mapped to a nonzero value
Timepix3Instrument::Timepix3Instrument(struct Counters &counters,
                               BaseSettings &settings)
    : counters(counters), Settings(settings) {

  XTRACE(INIT, ALW, "Loading configuration file %s",
         Settings.ConfigFile.c_str());
  Timepix3Configuration = Config(Settings.ConfigFile);

  Geom = new Geometry();
  Geom->ESSGeom = new ESSGeometry(Timepix3Configuration.XResolution, Timepix3Configuration.YResolution,1,1);


  // if (not Settings.DumpFilePrefix.empty()) {
  //   if (boost::filesystem::path(Settings.DumpFilePrefix).has_extension()) {

  //     DumpFile =
  //         ReadoutFile::create(boost::filesystem::path(Settings.DumpFilePrefix)
  //                                 .replace_extension(""));
  //   } else {
  //     DumpFile =
  //         ReadoutFile::create(Settings.DumpFilePrefix + "_" + timeString());
  //   }
  // }
 }

Timepix3Instrument::~Timepix3Instrument() {}

/// \brief helper function to calculate pixels from knowledge about
/// timepix3 panel, FENId and a single readout dataset
///
/// also applies the calibration
uint32_t Timepix3Instrument::calcPixel(DataParser::Timepix3Readout &Data) {
  XTRACE(DATA, DEB, "Calculating pixel");

  uint32_t pixel = Geom->calcPixel(Data);
  XTRACE(DATA, DEB, "Calculated pixel to be %u", pixel);
  return pixel;
}

// TODO, fix this
// void Timepix3Instrument::dumpReadoutToFile(DataParser::Timepix3Readout &Data) {
//   Readout CurrentReadout;
//   CurrentReadout.PulseTimeHigh = ESSReadoutParser.Packet.HeaderPtr->PulseHigh;
//   CurrentReadout.PulseTimeLow = ESSReadoutParser.Packet.HeaderPtr->PulseLow;
//   CurrentReadout.PrevPulseTimeHigh =
//       ESSReadoutParser.Packet.HeaderPtr->PrevPulseHigh;
//   CurrentReadout.PrevPulseTimeLow =
//       ESSReadoutParser.Packet.HeaderPtr->PrevPulseLow;
//   CurrentReadout.EventTimeHigh = Data.TimeHigh;
//   CurrentReadout.EventTimeLow = Data.TimeLow;
//   CurrentReadout.DataSeqNum = Data.DataSeqNum;
//   CurrentReadout.OutputQueue = ESSReadoutParser.Packet.HeaderPtr->OutputQueue;
//   CurrentReadout.AmpA = Data.AmpA;
//   CurrentReadout.AmpB = Data.AmpB;
//   CurrentReadout.AmpC = Data.AmpC;
//   CurrentReadout.AmpD = Data.AmpD;
//   CurrentReadout.RingId = Data.RingId;
//   CurrentReadout.FENId = Data.FENId;
//   CurrentReadout.TubeId = Data.TubeId;
//   DumpFile->push(CurrentReadout);
// }

void Timepix3Instrument::processReadouts() {

  /// Traverse readouts, calculate pixels
  for (auto &Data : Timepix3Parser.Result) {
    bool validData = Geom->validateData(Data);
    if (not validData) {
      XTRACE(DATA, WAR, "Invalid Data, skipping readout");
      continue;
    }

    // if (DumpFile) {
    //   dumpReadoutToFile(Data);
    // }

    // Calculate TOF in ns
    uint64_t TimeOfFlight = 0; /// todo, fix this


    // Calculate pixelid and apply calibration
    uint32_t PixelId = calcPixel(Data);

    if (PixelId == 0) {
      XTRACE(DATA, ERR, "Pixel error");
      counters.PixelErrors++;
    } else {
      XTRACE(DATA, DEB, "Valid data, adding to serializer");
      Serializer->addEvent(TimeOfFlight, PixelId);
      counters.Events++;
    }

  } // for()
}

} // namespace Timepix3
