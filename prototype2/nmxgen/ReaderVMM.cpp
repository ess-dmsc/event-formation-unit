/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <cstring>
#include <nmxgen/ReaderVMM.h>
//#include <nmxgen/Eventlet.h>

ReaderVMM::ReaderVMM(std::string filename) {
  data.resize(4, 0);
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

  size_t psize = sizeof(uint32_t) * 4;

  size_t limit = std::min(current_ + (9000 / psize), total_);
  size_t byteidx = 0;
  for (; current_ < limit; ++current_) {
    index[0] = current_;
    dataset_.read(data, slabsize, index);

    memcpy(buf, data.data(), psize);

    buf += psize;
    byteidx += psize;
  }
  return byteidx;
}
