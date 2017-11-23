/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cstring>
#include <iostream>
#include <gdgem/nmxgen/ReaderVMM.h>

ReaderVMM::ReaderVMM(std::string filename) {
  data.resize(max_in_buf_ * 4, 0);
  if (filename.empty())
    return;

  try {
    file_ = hdf5::file::open(filename);
    hdf5::node::Group root = file_.root();

    dataset_ = hdf5::node::get_dataset(root, "RawVMM/points");

    auto shape = hdf5::dataspace::Simple(dataset_.dataspace()).current_dimensions();

    if ((shape.size() != 2) || (shape[1] != 4))
      return;

    total_ = shape[0];
  }
  catch (std::exception& e)
  {
    std::cout << "Failed to open " << filename << " because:\n"
              << hdf5::error::print_nested(e) << std::endl;
  }
}

size_t ReaderVMM::read(char *buf) {
  size_t limit = std::min(current_ + max_in_buf_, total_);
  slab_.block(0, limit - current_);
  slab_.offset(0, current_);

   if (slab_.block()[0] > 0)
  {
    try {
      dataset_.read(data, slab_);
    }
    catch (std::exception& e)
    {
      std::cout << "Failed to read VMM slab because:\n"
                << hdf5::error::print_nested(e) << std::endl;
    }
    memcpy(buf, data.data(), psize_ * slab_.block()[0]);
  }

  current_ = limit;
  return psize_ * slab_.block()[0];
}
