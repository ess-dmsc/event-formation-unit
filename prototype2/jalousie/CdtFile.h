/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <jalousie/Readout.h>
#include <fstream>

namespace Jalousie {

class CdtFile {
public:
  CdtFile(const boost::filesystem::path &FilePath);

//  size_t count() const;

//  void readAt(size_t Index, size_t Count);
  size_t read();

  static constexpr size_t ChunkSize{9000 / sizeof(Readout)};

  std::vector<Readout> Data;

  struct stats_t {
    uint64_t events_found;
    uint64_t pulses_found;
  } stats;

private:
  size_t MaxSize{0};
  std::ifstream file_;

  void surveyFile();

};

}