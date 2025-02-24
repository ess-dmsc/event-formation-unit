// Copyright (C) 2016 - 2025 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief VMM/HDF5 'Hits' reader
///
//===----------------------------------------------------------------------===//
// GCOVR_EXCL_START

#include <generators/udpgen_hits/ReaderHits.h>

namespace Gem {

ReaderHits::ReaderHits(const std::string &filename) {
  file = HitFile::open(filename);
  total_ = file->count();
  ReadoutSize = sizeof(Hit);
  ChunkSize = HitFile::ChunkSize;
  current_ = 0;
}

size_t ReaderHits::read(char *buf) {
  size_t size = HitFile::ChunkSize;
  if ((current_ + HitFile::ChunkSize) > total_) {
    size = total_ - current_;
  }

  if (size > 0) {
    try {
      file->readAt(current_, size);
      memcpy(buf, file->Data.data(), sizeof(Hit) * size);
    } catch (std::exception &e) {
      std::cout << "<ReaderHits> failed to read slab (" << current_ << ", "
                << (current_ + size) << ")"
                << " max=" << total_ << "\n"
                << hdf5::error::print_nested(e, 1) << std::endl;
    }
  }

  current_ += size;
  return sizeof(Hit) * size;
}

} // namespace Gem
// GCOVR_EXCL_STOP
