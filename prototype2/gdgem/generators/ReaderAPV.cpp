/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/generators/ReaderAPV.h>
#include <iostream>
//#include <cstring>

ReaderVMM::ReaderVMM(std::string filename) {
  data.resize(max_in_buf_ * 4, 0);
  slab_.block(0, max_in_buf_);

  if (filename.empty())
    return;

  try {
    file_ = hdf5::file::open(filename);
    dataset_ = file_.root().get_dataset("RawVMM/points");

    auto shape = hdf5::dataspace::Simple(dataset_.dataspace()).current_dimensions();
    if ((shape.size() != 2) || (shape[1] != 4))
      throw std::runtime_error("ReaderVMM: wrong dimensions for RawVMM/points");

    total_ = shape[0] - 1;
    std::cout << "<ReaderVMM> opened " << filename << " with " << total_
              << " events, should produce ~" << (total_ / max_in_buf_)
              << " buffers\n";
  } catch (std::exception &e) {
    std::cout << "<ReaderVMM> failed to open " << filename << "\n"
              << hdf5::error::print_nested(e, 1) << std::endl;
  }
}

size_t ReaderVMM::read(char *buf) {
  slab_.offset(0, current_);
  if ((current_ + max_in_buf_) > total_)
  {
    slab_.block(0, total_ - current_);
    data.resize(slab_.block()[0] * 4, 0);
  }

  if (slab_.block()[0] > 0) {
    try {
      dataset_.read(data, slab_);
      memcpy(buf, data.data(), psize_ * slab_.block()[0]);
    } catch (std::exception &e) {
      std::cout << "<ReaderVMM> failed to read VMM slab ("
                << current_ << ", " << (current_ + slab_.block()[0]) << ")"
                << " max=" << total_ << "\n"
                << hdf5::error::print_nested(e, 1) << std::endl;
    }
  }

  current_ += slab_.block()[0];
  return psize_ * slab_.block()[0];
}
