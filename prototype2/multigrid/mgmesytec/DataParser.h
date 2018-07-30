/** Copyright (C) 2016-2018 European Spallation Source */
//===----------------------------------------------------------------------===//
///
/// \file
/// Class to parse detector readout for multigrid via
/// sis3153 / Mesytec digitizer
///
//===----------------------------------------------------------------------===//

#pragma once
#include <common/FBSerializer.h>
#include <common/Hists.h>
#include <common/ReadoutSerializer.h>
#include <logical_geometry/ESSGeometry.h>
#include <multigrid/mgmesytec/Vmmr16Parser.h>

class MesytecData {
public:
  enum class error { OK = 0, ESIZE, EHEADER, EUNSUPP };

  /// \brief if it looks like a constructor...
  MesytecData(MgEFU mg_efu, std::shared_ptr<ReadoutSerializer> s, std::string fileprefix = "");

  ~MesytecData() = default;

  uint32_t getPixel(); // \todo (too) simple implm. but agreed for now
  uint32_t getTime();  // \todo (too) simple implm. but agreed for now

  /** \brief parse a binary payload buffer, return number of data element
   * \todo Uses NMXHists  - refactor and move ?
   */
  error parse(const char *buffer, int size, FBSerializer &fbserializer);

  // Statistics updated by parse()
  MgStats stats;

  uint64_t RecentPulseTime{0};

private:
  VMMR16Parser vmmr16Parser;

  // \todo deduce this from mappings
  ESSGeometry Geometry{36, 40, 20, 1};

  std::shared_ptr<MGHitFile> dumpfile;

  // \todo factor this out, common with gdgem
  static std::string time_str();
};
