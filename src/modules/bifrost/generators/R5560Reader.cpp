// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief reader for raw bifrost detector data
///
/// Raw data format gotten from Paolo Mutti
///
//===----------------------------------------------------------------------===//

#include <assert.h>
#include <bifrost/generators/R5560Reader.h>
#include <bifrost/readout/Readout.h>
#include <fcntl.h>
#include <string>

// GCOVR_EXCL_START

R5560Reader::R5560Reader(std::string file) : filename(file) {
  fd = open(filename.c_str(), O_RDONLY);
}


R5560Reader::~R5560Reader() {
  close(fd);
}


int R5560Reader::readReadout(struct raw_data_t &Readout) {
  int res = read(fd, &Readout, sizeof(struct raw_data_t));

  if (res != sizeof(struct raw_data_t)) {
    printf("read returned %d\n", res);
    return -1;
  }

  readouts++;
  return 1;
}

// GCOVR_EXCL_STOP
