/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Wrapper class for reading VMM data from HDF5 files
 */

#pragma once

#include <gdgem/nmx/EventletFile.h>

class ReaderEventlets {
public:
  /** @todo document */
  ReaderEventlets(std::string filename);

  /** @todo document */
  size_t read(char *buf);

private:
  EventletFile file;

  size_t total_{0};
  size_t current_{0};
};
