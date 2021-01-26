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

#include <loki/generators/RawReader.h>
#include <loki/readout/DataParser.h>
#include <assert.h>
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <stdexcept>

// GCOVR_EXCL_START

LokiReader::LokiReader(std::string file) : filename(file) {
  fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1) {
    throw std::runtime_error("open() failed - check file and permissions");
  }
}

int LokiReader::read32(uint32_t & val) {
  return (int)read(fd, &val, 4);
}

int LokiReader::read16(uint16_t & val) {
  return (int)read(fd, &val, 2);
}

int LokiReader::read8(uint8_t & val) {
  return (int)read(fd, &val, 1);
}

void LokiReader::skip(int n) {
  uint8_t ignored;
  for (int i = 0; i < n; i++) {
    read(fd, &ignored, 1);
  }
}

int LokiReader::readReadout(struct Loki::DataParser::LokiReadout & Readout) {
  raw_data_t rawdata;

  int res = read(fd, &rawdata, sizeof(struct raw_data_t));
  if (rawdata.cookie != 0xDA71DEDA) {
    printf("No cookie in data return -1 (and exit)\n");
    return -1;
  }

  if (res != -1) {
    Readout.TimeHigh = rawdata.tof1;
    Readout.TimeHigh = rawdata.tof2;
    Readout.TubeId = rawdata.tube;
    Readout.unused = 0x00; // Operating Mode eventually
    Readout.DataSeqNum = 0x0000;
    Readout.AmpA = rawdata.a;
    Readout.AmpB = rawdata.b;
    Readout.AmpC = rawdata.c;
    Readout.AmpD = rawdata.d;
  }
  return res;
}

// GCOVR_EXCL_STOP
