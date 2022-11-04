// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Separating Miracles processing from pipeline main loop
///
/// Holds efu stats, instrument readout mappings, logical geometry, pixel
/// calculations and Miracles readout parser
//===----------------------------------------------------------------------===//

#pragma once

#include <miracles/MiraclesBase.h> // to get MiraclesSettings
#include <miracles/Counters.h>
#include <miracles/geometry/Config.h>
#include <miracles/geometry/Geometry.h>
#include <miracles/readout/DataParser.h>
#include <miracles/readout/Readout.h>
#include <common/readout/ess/ESSTime.h>
#include <common/readout/ess/Parser.h>
#include <logical_geometry/ESSGeometry.h>

namespace Miracles {

class MiraclesInstrument {
public:
  /// \brief 'create' the LoKI instruments
  ///
  /// loads configuration and calibration files, calulate and generate the
  /// logical geometry and initialise the amplitude to position calculations
  MiraclesInstrument(Counters &counters, BaseSettings & Settings);

  ~MiraclesInstrument();

  //
  void processReadouts();

  //
  void setSerializer(EV42Serializer *serializer) { Serializer = serializer; }

  /// \brief Miracles pixel calculations
  uint32_t calcPixel(int Ring, int Tube, int AmpA, int AmpB);

  /// \brief writes a single readout to file
  void dumpReadoutToFile(DataParser::MiraclesReadout &Data);

public:
  /// \brief Stuff that 'ties' Miracles together
  struct Counters &counters;
  BaseSettings &Settings;
  Config MiraclesConfiguration;
  ESSGeometry lgeom{48, 128, 1, 1};
  Geometry geom;
  // Calibration MiraclesCalibration;
  ESSReadout::Parser ESSReadoutParser;
  DataParser MiraclesParser{counters};
  EV42Serializer *Serializer{nullptr};
  std::shared_ptr<ReadoutFile> DumpFile;
};

} // namespace Miracles
