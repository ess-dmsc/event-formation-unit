/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for reading VMM data from HDF5 files
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <gdgem/nmx/Readout.h>

namespace Gem {

class ReaderReadouts {
public:
  /// \todo document
  ReaderReadouts(std::string filename);

  /// \todo document
  size_t read(char *buf);

  ///
  size_t getReadoutSize() const { return ReadoutSize;}

  ///
  size_t getChunkSize() const { return ChunkSize;}

private:
  std::shared_ptr<ReadoutFile> file;

  size_t total_{0};
  size_t current_{0};
  size_t ReadoutSize{0};
  size_t ChunkSize{0};
};

}

// GCOVR_EXCL_STOP
