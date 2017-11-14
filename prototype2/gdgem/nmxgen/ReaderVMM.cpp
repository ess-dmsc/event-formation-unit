/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cstring>
#include <iostream>
#include <gdgem/nmxgen/ReaderVMM.h>

ReaderVMM::ReaderVMM(std::string filename) {
  data.resize(max_in_buf_ * 4, 0);
  if (filename.empty())
    return;
  file_ = hdf5::file::open(filename);
  hdf5::node::Group root = file_.root();
  if (!root.exists("RawVMM") ||
      !hdf5::node::Group(root["RawVMM"]).exists("points"))
    return;

//  file_ = H5CC::File(filename, H5CC::Access::r_existing);
//  if (!file_.has_group("RawVMM") ||
//      !file_.open_group("RawVMM").has_dataset("points"))
//    return;

  dataset_ = hdf5::node::Dataset(root["RawVMM/points"]);

//  dataset_ = file_.open_group("RawVMM").open_dataset("points");

  auto shape = hdf5::dataspace::Simple(dataset_.dataspace()).current_dimensions();

  if ((shape.size() != 2) || (shape[1] != 4))
    return;

//  auto shape = dataset_.shape();
//  if ((shape.rank() != 2) || (shape.dim(1) != 4))
//    return;

  total_ = shape[0];
//  total_ = shape.dim(0);
}

size_t ReaderVMM::read(char *buf) {
  size_t limit = std::min(current_ + max_in_buf_, total_);
  slab_.block(0, limit - current_);
  slab_.offset(0, current_);

//  slabsize[0] = (limit - current_);
//  index[0] = current_;

  if (slab_.block()[0] > 0)
//  if (slabsize[0] > 0)
  {
    dataset_.read(data, slab_);
    memcpy(buf, data.data(), psize_ * slab_.block()[0]);
//    dataset_.read(data, slabsize, index);
//    memcpy(buf, data.data(), psize_ * slabsize[0]);
  }

  current_ = limit;
  return psize_ * slab_.block()[0];
//  return psize_ * slabsize[0];
}
