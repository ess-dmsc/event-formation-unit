/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for reading VMM data from HDF5 files
///
//===----------------------------------------------------------------------===//

#pragma once

#include <gdgem/nmx/HitFile.h>

class ReaderHits {
public:
  /** @todo document */
  ReaderHits(std::string filename);

  /** @todo document */
  size_t read(char *buf);

private:
  HitFile file;

  size_t total_{0};
  size_t current_{0};
};
