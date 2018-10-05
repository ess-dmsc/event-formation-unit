/** Copyright (C) 2016, 2017 European Spallation Source ERIC */
// GCOVR_EXCL_START

#include <gdgem/generators/ReaderHits.h>
#include <iostream>

ReaderHits::ReaderHits(std::string filename) {
  file.open_r(filename);
  total_ = file.count();
  current_ = 0;
}

size_t ReaderHits::read(char *buf) {
  size_t size = HitFile::chunk_size;
  if ((current_ + HitFile::chunk_size) > total_)
  {
    size = total_ - current_;
  }

  if (size > 0) {
    try {
      file.read_at(current_, size);
      memcpy(buf, file.data.data(), sizeof(Hit) * size);
    } catch (std::exception &e) {
      std::cout << "<ReaderHits> failed to read slab ("
                << current_ << ", " << (current_ + size) << ")"
                << " max=" << total_ << "\n"
                << hdf5::error::print_nested(e, 1) << std::endl;
    }
  }

  current_ += size;
  return sizeof(Hit) * size;
}
// GCOVR_EXCL_STOP
