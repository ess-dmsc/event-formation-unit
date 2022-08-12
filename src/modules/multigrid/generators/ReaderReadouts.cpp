/** Copyright (C) 2016, 2017 European Spallation Source ERIC */
// GCOVR_EXCL_START

#include <iostream>
#include <multigrid/generators/ReaderReadouts.h>

namespace Multigrid {

ReaderReadouts::ReaderReadouts(std::string filename) {
  file = ReadoutFile::open(filename);
  total_ = file->count();
  ReadoutSize = sizeof(Readout);
  ChunkSize = ReadoutFile::ChunkSize;
  current_ = 0;
}

size_t ReaderReadouts::read(char *buf) {
  size_t size = ReadoutFile::ChunkSize;
  if ((current_ + ReadoutFile::ChunkSize) > total_) {
    size = total_ - current_;
  }

  if (size > 0) {
    try {
      file->readAt(current_, size);
      memcpy(buf, file->Data.data(), sizeof(Readout) * size);
    } catch (std::exception &e) {
      std::cout << "<ReaderReadouts> failed to read slab (" << current_ << ", "
                << (current_ + size) << ")"
                << " max=" << total_ << "\n"
                << hdf5::error::print_nested(e, 1) << std::endl;
    }
  }

  current_ += size;
  return sizeof(Readout) * size;
}

} // namespace Multigrid

// GCOVR_EXCL_STOP
