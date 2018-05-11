/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/generators/ReaderEventlets.h>
#include <iostream>
//#include <cstring>

ReaderEventlets::ReaderEventlets(std::string filename) {
  data.resize(max_in_buf_);
  slab_.block(0, max_in_buf_);

  if (filename.empty())
    return;

  try {
    file_ = hdf5::file::open(filename);
    dataset_ = file_.root().get_dataset("h5eventlets");

    auto shape = hdf5::dataspace::Simple(dataset_.dataspace()).current_dimensions();
    if (shape.size() != 1)
      throw std::runtime_error("ReaderEventlets: wrong dimensions for h5eventlets");

    total_ = shape[0] - 1;
    std::cout << "<ReaderEventlets> opened " << filename << " with " << total_
              << " events, should produce ~" << (total_ / max_in_buf_)
              << " buffers\n";
  } catch (std::exception &e) {
    std::cout << "<ReaderEventlets> failed to open " << filename << "\n"
              << hdf5::error::print_nested(e, 1) << std::endl;
  }
}

size_t ReaderEventlets::read(char *buf) {
  slab_.offset(0, current_);
  if ((current_ + max_in_buf_) > total_)
  {
    slab_.block(0, total_ - current_);
    data.resize(slab_.block()[0]);
  }

  if (slab_.block()[0] > 0) {
    try {
      dataset_.read(data, slab_);
      memcpy(buf, data.data(), sizeof(Eventlet) * slab_.block()[0]);
    } catch (std::exception &e) {
      std::cout << "<ReaderEventlets> failed to read slab ("
                << current_ << ", " << (current_ + slab_.block()[0]) << ")"
                << " max=" << total_ << "\n"
                << hdf5::error::print_nested(e, 1) << std::endl;
    }
  }

  current_ += slab_.block()[0];
  return sizeof(Eventlet) * slab_.block()[0];
}
