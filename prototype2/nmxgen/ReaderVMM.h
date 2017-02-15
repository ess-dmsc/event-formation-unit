/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Wrapper class for reading VMM data from HDF5 files
 */

#ifndef READER_VMM_H
#define READER_VMM_H

#include <h5cc/H5CC_DataSet.h>
#include <h5cc/H5CC_File.h>
#include <map>
#include <vector>

#include <nmxgen/EventNMX.h>

struct PacketVMM {
  uint32_t time_offset;
  uint16_t timebin;
  uint8_t plane_id;
  uint8_t strip;
  uint16_t adc;
};

struct c2d {
  c2d(uint32_t xx, uint32_t yy) : x(xx), y(yy) {}
  uint32_t x, y;
};

inline bool operator<(const c2d &a, const c2d &b) {
  return a.x < b.x || (!(b.x < a.x) && a.y < b.y);
}

using HistMap2D = std::map<c2d, double>;

bool write(H5CC::Group group, std::string name, const HistMap2D &hist,
           uint16_t subdivisions = 10);

class ReaderVMM {
public:
  ReaderVMM(std::string filename);

  size_t read(char *buf);

private:
  H5CC::File file_;
  H5CC::DataSet dataset_;

  size_t total_{0};
  size_t current_{0};

  const std::vector<hsize_t> slabsize{1, H5CC::kMax};
  std::vector<hsize_t> index{0, 0};
};

#endif
