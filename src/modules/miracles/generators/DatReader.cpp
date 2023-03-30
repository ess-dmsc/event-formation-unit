// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief reader for MIRACLES data
///
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <caen/readout/Readout.h>
#include <fstream>
#include <miracles/generators/DatReader.h>
#include <sstream>
#include <string>

// GCOVR_EXCL_START

MiraclesDatReader::MiraclesDatReader(std::string file, bool Verbose)
    : filename(file), Verbose(Verbose) {
  infile = new std::ifstream(filename);
}

int MiraclesDatReader::readReadout(struct dat_data_t &Readout) {
  std::string line;

  // Ignore first line
  if (lines == 0) {
    std::getline(*infile, line);
    lines++;
  }

  if (std::getline(*infile, line)) {
    std::istringstream iss(line);
    std::string ring, fen, tofhi, toflow, tube, ampl_a, ampl_b;

    iss >> ring >> fen >> tofhi >> toflow >> tube >> ampl_a >> ampl_b;
    Readout.ring = stoi(ring);
    Readout.fen = stoi(fen);
    Readout.tofhi = stoi(tofhi);
    Readout.toflow = stoi(toflow);
    Readout.tube = stoi(tube);
    Readout.ampl_a = stoi(ampl_a);
    Readout.ampl_b = stoi(ampl_b);
    Readout.unused1 = 0;
    Readout.unused2 = 0;
    Readout.unused3 = 0;

    if (Verbose) {
      printf("ring %2u, fen %2u, time (%5u, %7u) - tube %2u, A: %5u, B: %5u\n",
             Readout.ring, Readout.fen, Readout.tofhi, Readout.toflow,
             Readout.tube, Readout.ampl_a, Readout.ampl_b);
    }
    lines++;
    return 1;
  } else {
    if (Verbose) {
      printf("EOF\n");
    }
  }
  return -1;
}

// GCOVR_EXCL_STOP
