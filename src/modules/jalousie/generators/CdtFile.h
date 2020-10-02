// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
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

  size_t total() const;
  size_t read();
  size_t read(char *buffer);

  size_t getReadoutSize() const { return ReadoutSize; }
  size_t getChunkSize() const { return ChunkSize; }

  std::vector<Readout> Data;

  struct survey_results_t {
    size_t unidentified_board {0};
    size_t events_found {0};
    size_t metadata_found {0};
    size_t adc_found {0};
    size_t board_blocks {0};
    size_t chopper_pulses {0};
  } survey_results;

private:
  size_t ReadoutSize{sizeof(Readout)};
  size_t ChunkSize{9000 / sizeof(Readout)};

  size_t readouts_expected {0};
  std::ifstream file_;
  bool have_board {false};
  uint64_t data{0};
  Readout readout;
  bool read_one();
};

}