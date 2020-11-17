// Copyright (C) 2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Multigrid processing from pipeline main loop
///
//===----------------------------------------------------------------------===//

#include <common/Log.h>
#include <common/Trace.h>
#include <common/TimeString.h>
#include <multiblade/MBCaenInstrument.h>

// #undef TRC_LEVEL
// #define TRC_LEVEL TRC_L_DEB

namespace Multiblade {

/// \brief load configuration and calibration files
MBCaenInstrument::MBCaenInstrument(struct Counters & counters,
    CAENSettings &moduleSettings)
      : counters(counters)
      , ModuleSettings(moduleSettings) {

    MultibladeConfig = Config(ModuleSettings.ConfigFile);
    assert(MultibladeConfig.getDigitizers() != nullptr);

    ncass = MultibladeConfig.getCassettes();
    nwires = MultibladeConfig.getWires();
    nstrips = MultibladeConfig.getStrips();

    histograms = Hists(std::max(ncass * nwires, ncass * nstrips), 65535);

    builders = std::vector<EventBuilder>(ncass);

    mbgeom = MBGeometry(ncass, nwires, nstrips);
    if (MultibladeConfig.getInstrument() == Config::InstrumentGeometry::Estia) {
      XTRACE(PROCESS, ALW, "Setting instrument configuration to Estia");
      mbgeom.setConfigurationEstia();
      essgeom = ESSGeometry(ncass * nwires, nstrips, 1, 1);
      topic = "ESTIA_detector";
      monitor = "ESTIA_monitor";
    } else {
      mbgeom.setConfigurationFreia();
      XTRACE(PROCESS, ALW, "Setting instrument configuration to Freia");
      essgeom = ESSGeometry(nstrips, ncass * nwires, 1, 1);
      topic = "FREIA_detector";
      monitor = "FREIA_monitor";
    }

    if (MultibladeConfig.getDetectorType() == Config::DetectorType::MB18) {
      XTRACE(PROCESS, ALW, "Setting detector to MB18");
      mbgeom.setDetectorMB18();
    } else {
      XTRACE(PROCESS, ALW, "Setting detector to MB16");
      mbgeom.setDetectorMB16();
    }
}



void MBCaenInstrument::ingestOneReadout(int cassette, const Readout & dp) {

  if (not mbgeom.isValidCh(dp.channel)) {
    counters.ReadoutsInvalidChannel++;
    return;
  }

  if (dp.adc > MultibladeConfig.max_valid_adc) {
    counters.ReadoutsInvalidAdc++;
    return;
  }

  uint8_t plane = mbgeom.getPlane(dp.channel);
  uint16_t global_ch = mbgeom.getGlobalChannel(cassette, dp.channel);
  uint16_t coord;
  if (plane == 0) {
    if (global_ch == 30) {
      counters.ReadoutsMonitor++;
      return;
    }
    coord = mbgeom.getx(cassette, dp.channel);
    histograms.bin_x(global_ch, dp.adc);
  } else  if (plane == 1) {
    coord = mbgeom.gety(cassette, dp.channel);
    histograms.bin_y(global_ch, dp.adc);
  } else {
    counters.ReadoutsInvalidPlane++;
    return;
  }

  counters.ReadoutsGood++;

  XTRACE(DATA, DEB, "time %lu, channel %u, adc %u", dp.local_time, dp.channel, dp.adc);

  builders[cassette].insert({dp.local_time, coord, dp.adc, plane});

  XTRACE(DATA, DEB, "Readout (%s) -> cassette=%d plane=%d coord=%d",
         dp.debug().c_str(), cassette, plane, coord);
  }

} // namespace
