/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/generators/ReaderEventlets.h>
#include <iostream>

ReaderEventlets::ReaderEventlets(std::string filename) {
  file.open_r(filename);
  total_ = file.count();
  current_ = 0;
}

size_t ReaderEventlets::read(char *buf) {
  size_t size = EventletFile::chunk_size;
  if ((current_ + EventletFile::chunk_size) > total_)
  {
    size = total_ - current_;
  }

  if (size > 0) {
    try {
      file.read_at(current_, size);
      memcpy(buf, file.data.data(), sizeof(Eventlet) * size);
    } catch (std::exception &e) {
      std::cout << "<ReaderEventlets> failed to read slab ("
                << current_ << ", " << (current_ + size) << ")"
                << " max=" << total_ << "\n"
                << hdf5::error::print_nested(e, 1) << std::endl;
    }
  }

  current_ += size;
  return sizeof(Eventlet) * size;
}
