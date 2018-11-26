/** Copyright (C) 2016, 2017 European Spallation Source ERIC */
// GCOVR_EXCL_START

#include <gdgem/generators/ReaderHits.h>
#include <iostream>

ReaderHits::ReaderHits(std::string filename) {
  file = Gem::HitFile::open(filename);
  total_ = file->count();
  current_ = 0;
}

size_t ReaderHits::read(char *buf) {
  size_t size = Gem::HitFile::ChunkSize;
  if ((current_ + Gem::HitFile::ChunkSize) > total_)
  {
    size = total_ - current_;
  }

  if (size > 0) {
    try {
      file->readAt(current_, size);
      memcpy(buf, file->Data.data(), sizeof(Gem::Hit) * size);
    } catch (std::exception &e) {
      std::cout << "<ReaderHits> failed to read slab ("
                << current_ << ", " << (current_ + size) << ")"
                << " max=" << total_ << "\n"
                << hdf5::error::print_nested(e, 1) << std::endl;
    }
  }

  current_ += size;
  return sizeof(Gem::Hit) * size;
}
// GCOVR_EXCL_STOP
