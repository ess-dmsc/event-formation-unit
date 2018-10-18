/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mbcaen/MBData.h>
#include <memory>

namespace Multiblade {

MBData::MBData() : data(0) {}

unsigned int MBData::receive(const char *buffer, unsigned int size) {

  data.clear();

  size_t digi_size = sizeof(datapoint::digi);
  size_t chan_size = sizeof(datapoint::chan);
  size_t adc_size = sizeof(datapoint::adc);
  size_t time_size = sizeof(datapoint::time);

  size_t linesize = digi_size + chan_size + adc_size + time_size;

  size_t read_data = 0;

  const char *bufptr = buffer;

  datapoint dp;

  do {
    std::memcpy(&dp.digi, bufptr, digi_size);
    bufptr += digi_size;

    std::memcpy(&dp.chan, bufptr, chan_size);
    bufptr += chan_size;

    std::memcpy(&dp.adc, bufptr, adc_size);
    bufptr += adc_size;

    std::memcpy(&dp.time, bufptr, time_size);
    bufptr += time_size;

    data.push_back(dp);

    read_data += linesize;
  } while (read_data < size);

  return data.size();
}

}
