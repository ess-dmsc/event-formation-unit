// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief reader for Bifrost data
///
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <bifrost/generators/ReadoutGenerator.h>
#include <caen/readout/Readout.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>

// GCOVR_EXCL_START

ReadoutGenerator::ReadoutGenerator(std::string file, bool Verbose)
    : filename(file), Verbose(Verbose) {
  FileDescriptor = open(file.c_str(), O_RDONLY);
}

int ReadoutGenerator::readReadout(struct dat_data_t &Readout) {

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

void ReadoutGenerator::generateData() {
  uint64_t SentPackets = 0;
  uint64_t SentReadouts = 0;

  struct dat_data_t DatReadout;
  struct udp_data_t UdpReadout;
  int res = 0;

  while (((res = readReadout(DatReadout)) > 0) and
         (SentPackets < Settings.NumberOfPackets) and
         (SentReadouts < Settings.NumReadouts)) {

    memset(&UdpReadout, 0, sizeof(UdpReadout));
    
    UdpReadout.timehi = ;
    UdpReadout.timelow = DatReadout.timelow;
    UdpReadout.flagsAndOM = 0;
    UdpReadout.group = DatReadout.tube;
    UdpReadout.ampl_a = DatReadout.ampl_a;
    UdpReadout.ampl_b = DatReadout.ampl_b;
  }
}

// GCOVR_EXCL_STOP
