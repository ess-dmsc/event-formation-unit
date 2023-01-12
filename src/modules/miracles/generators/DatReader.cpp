// Copyright (C) 2023 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief reader for MIRACLES data
///
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <miracles/generators/DatReader.h>
#include <caen/readout/Readout.h>
#include <fstream>
#include <sstream>
#include <string>

// GCOVR_EXCL_START

MiraclesDatReader::MiraclesDatReader(std::string file) : filename(file) {
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
    std::string tofhi, toflow, tube, ampl_a, ampl_b;

    iss >> tofhi >> toflow >> tube >> ampl_a >> ampl_b;
    Readout.tofhi = stoi(tofhi);
    Readout.toflow = stoi(toflow);
    Readout.tube = stoi(tube);
    Readout.ampl_a = stoi(ampl_a);
    Readout.ampl_b = stoi(ampl_b);

    printf("time (%u, %u) - tube %u, amplitudes A: %u, B: %u\n",
        Readout.tofhi, Readout.toflow, Readout.tube, Readout.ampl_a,
        Readout.ampl_b);
    lines++;
    return 1;
  } else {
    printf("EOF\n");
  }
  return -1;
}

// GCOVR_EXCL_STOP
