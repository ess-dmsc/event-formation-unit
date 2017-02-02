/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <nmxgen/ReaderVMM.h>
#include <cstring>

ReaderVMM::ReaderVMM(std::string filename) 
{
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

size_t ReaderVMM::read(char *buf) 
{
  // Event is timeoffset(32) timebin(16) planeID(8) stripnum(8) ADC(16)
  //  total = 10 bytes, unsigned
  // this should be quite close to the size of final data format

  PacketVMM packet;

  size_t limit = std::min(current_ + (9000/12), total_);
  size_t byteidx = 0;
  for (; current_ < limit; ++current_) 
  {
    index[0] = current_;
    auto data = dataset_.read<uint32_t>(slabsize, index);

    packet.time_offset = data.at(0);
    packet.timebin = static_cast<uint16_t>(data.at(1));
    packet.plane_id = (data.at(2) >> 16);
    packet.strip = static_cast<uint8_t>(data.at(2) & 0x000000FF);
    packet.adc = static_cast<uint16_t>(data.at(3));

    memcpy(buf, &packet, sizeof(packet));

    buf += 12;
    byteidx +=12;
  }
  return byteidx;
}


bool write(H5CC::Group group, std::string name,
           const HistMap2D& hist, uint16_t subdivisions)
{
  if (hist.empty() ||
      group.name().empty() ||
      name.empty() ||
      group.has_dataset(name))
    return false;

  uint32_t xmax {0};
  uint32_t ymax {0};
  for (auto d : hist)
  {
    xmax = std::max(xmax, d.first.x);
    ymax = std::max(ymax, d.first.y);
  }

  xmax++;
  ymax++;

  H5CC::DataSet dataset;
  
  std::cout << "Xmax=" << xmax << " Ymax=" << ymax << "\n";
  
  if (subdivisions > 1)
   dataset = group.create_dataset<double>(name, {xmax, ymax},
                                                {xmax/subdivisions, ymax/subdivisions});
  else
   dataset = group.create_dataset<double>(name, {xmax, ymax});
  
  for (auto d : hist)
    if (d.second)
      dataset.write<double>(d.second, {d.first.x, d.first.y});

  return true;
}
