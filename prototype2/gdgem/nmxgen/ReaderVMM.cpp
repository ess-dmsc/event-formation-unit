/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cstring>
#include <gdgem/nmxgen/ReaderVMM.h>

ReaderVMM::ReaderVMM(std::string filename) {
  data.resize(max_in_buf_ * 4, 0);
  if (filename.empty())
    return;
  file_ = H5CC::File(filename, H5CC::Access::r_existing);
  if (!file_.has_group("RawVMM") ||
      !file_.open_group("RawVMM").has_dataset("points"))
    return;
  dataset_ = file_.open_group("RawVMM").open_dataset("points");

  auto shape = dataset_.shape();
  if ((shape.rank() != 2) || (shape.dim(1) != 4))
    return;

  total_ = shape.dim(0);
}

size_t ReaderVMM::read(char *buf) {
  size_t limit = std::min(current_ + max_in_buf_, total_);
  slabsize[0] = (limit - current_);
  current_ = limit;
  index[0] = current_;
  dataset_.read(data, slabsize, index);
  memcpy(buf, data.data(), psize_ * slabsize[0]);
  return psize_ * slabsize[0];
}
