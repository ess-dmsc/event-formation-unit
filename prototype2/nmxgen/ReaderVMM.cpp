/** Copyright (C) 2016 European Spallation Source ERIC */

#include <nmxgen/ReaderVMM.h>

ReaderVMM::ReaderVMM(std::string filename) {
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
  // Event is timeoffset(32) timebin(16) planeID(8) stripnum(8) ADC(16)
  //  total = 10 bytes, unsigned
  // this should be quite close to the size of final data format

  size_t limit = std::min(current_ + 900, total_);
  size_t byteidx = 0;
  for (; current_ < limit; ++current_) {
    index[0] = current_;
    auto data = dataset_.read<uint32_t>(slabsize, index);
    buf[byteidx] = data.at(0); // time offset (32 bits)
    buf[byteidx + 4] = static_cast<uint16_t>(data.at(1)); // timebin (16 bits)
    buf[byteidx + 6] =
        static_cast<uint8_t>(data.at(2) >> 16); // planeid (8 bits)
    buf[byteidx + 7] =
        static_cast<uint8_t>(data.at(2) & 0x000000FF);    // strip   (8 bits)
    buf[byteidx + 8] = static_cast<uint16_t>(data.at(3)); // adc value (16 bits)
    byteidx += 10;
  }
  return byteidx;
}
