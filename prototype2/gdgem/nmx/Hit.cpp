/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <gdgem/nmx/Hit.h>
#include <sstream>

namespace Gem {

std::string Hit::debug() const {
  std::stringstream ss;
  //  ss << " time=" << (time >> 36) << ":" << (time & 0xFFFFFFFF);
//  ss << " time=" << (time >> 52) << ":" << ((time >> 20) & 0xFFFFFFFF) << ":"
//     << ((time >> 8) & 0xFFF) << ":" << (time & 0xFF);
  ss << " time=" << time;
  ss << " plane=" << plane_id << " strip=" << strip << " adc=" << adc;
  return ss.str();
}

}
