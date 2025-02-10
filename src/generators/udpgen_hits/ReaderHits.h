// Copyright (C) 2016 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Wrapper class for reading VMM data ('Hits') from HDF5 files
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#pragma once

#include <common/reduction/Hit.h>

namespace Gem {

class ReaderHits {
public:
  /// \todo document
  ReaderHits(const std::string &filename);

  /// \todo document
  size_t read(char *buf);

  ///
  size_t getReadoutSize() const { return ReadoutSize; }

  ///
  size_t getChunkSize() const { return ChunkSize; }

private:
  std::shared_ptr<HitFile> file;

  size_t total_{0};
  size_t current_{0};
  size_t ReadoutSize{0};
  size_t ChunkSize{0};
};

} // namespace Gem
// GCOVR_EXCL_STOP
