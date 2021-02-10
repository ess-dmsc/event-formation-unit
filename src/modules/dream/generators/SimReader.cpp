// Copyright (C) 2021 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief reader for raw loki detector data
///
/// Raw data format gotten from Davide Raspino at STFC
///
//===----------------------------------------------------------------------===//

#include <dream/generators/SimReader.h>
#include <dream/readout/Readout.h>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <string>

// GCOVR_EXCL_START

DreamSimReader::DreamSimReader(std::string file) : filename(file) {
  infile = new std::ifstream(filename);
}

int DreamSimReader::readReadout(struct sim_data_t & Readout) {
  std::string line;

  // Ignore first line
  if (lines == 0) {
    std::getline(*infile, line);
    lines++;
    return 0;
  }

  if (std::getline(*infile, line)) {
      std::istringstream iss(line);
      std::string stof, smodule, ssumo, sstrip, swire, ssegment, scounter;

      iss >> stof >> smodule >> ssumo >> sstrip >> swire >> ssegment >> scounter;
      Readout.tof = stoi(stof);
      Readout.module = stoi(smodule);
      Readout.sumo = stoi(ssumo);
      Readout.strip = stoi(sstrip);
      Readout.wire = stoi(swire);
      Readout.segment = stoi(ssegment);
      Readout.counter = stoi(scounter);

      // printf("%u: %u, %u, %u, %u, %u, %u\n",
      //  Readout.tof, Readout.module, Readout.sumo, Readout.strip,
      //  Readout.wire, Readout.segment, Readout.counter);
      lines++;
      return 1;
  } else {
    printf("Getline failed\n");
  }
  return -1;
}

// GCOVR_EXCL_STOP
