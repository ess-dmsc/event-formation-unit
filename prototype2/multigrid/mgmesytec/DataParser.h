/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class to parse detector readout for multigrid via
/// sis3153 / Mesytec digitizer
///
//===----------------------------------------------------------------------===//

#pragma once
#include <common/EV42Serializer.h>
#include <logical_geometry/ESSGeometry.h>
#include <multigrid/mgmesytec/Vmmr16Parser.h>
#include <multigrid/mgmesytec/HitFile.h>
#include <multigrid/mgmesytec/MgEFU.h>

class MesytecData {
public:
  enum class error { OK = 0, ESIZE, EHEADER, EUNSUPP };

  // \todo register callback instead of passing serializer?
  /// \brief parse a binary payload buffer, return number of data element
  error parse(const Buffer &buffer, MgStats& stats);

  std::vector<Buffer> buffers;
};

class MesytecEFU {
public:
  /// \brief if it looks like a constructor...
  MesytecEFU(std::shared_ptr<MgEFU> mg_efu, bool spoof_ht,
      std::shared_ptr<MGHitFile> dump = nullptr);


  void parse(const std::vector<Buffer>& buffers, EV42Serializer &EV42Serializer, MgStats& stats);

  /// \todo document
  uint32_t getPixel();

  /// \todo document
  uint32_t getTime();

  void set_geometry(ESSGeometry);

  uint64_t RecentPulseTime{0};

private:
  VMMR16Parser vmmr16Parser;

  std::shared_ptr<MgEFU> mgEfu;
  ESSGeometry Geometry;
  std::shared_ptr<MGHitFile> dumpfile;
};

