// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief reader for Bifrost data
///
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <bifrost/generators/DatReader.h>
#include <caen/readout/Readout.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>

// GCOVR_EXCL_START

BifrostDatReader::BifrostDatReader(std::string file, bool Verbose)
    : filename(file), Verbose(Verbose) {
  FileDescriptor = open(file.c_str(), O_RDONLY);
}

int BifrostDatReader::readReadout(struct dat_data_t &Readout) {

  while (read(FileDescriptor, (void *)&Readout, sizeof(struct dat_data_t)) >
         0) {

    if (Verbose) {
      printf("ring %2u, fen 0, time (%5u, %7u) - tube %2u, A: %5u, B: %5u\n",
             Readout.fiber, Readout.timehi, Readout.timelow, Readout.tube,
             Readout.ampl_a, Readout.ampl_b);
    }
    Readouts++;
    if (Readouts % 1000000 == 0) {
      printf("Count %" PRIu64 "\n", Readouts);
    }
    return 1;
  }
  printf("EOF\n");
  return -1;
}

// GCOVR_EXCL_STOP
