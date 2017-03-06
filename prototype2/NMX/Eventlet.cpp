#include <NMX/Eventlet.h>
#include <sstream>

std::string Eventlet::debug() const {
  std::stringstream ss;
  if (flag)
    ss << " flag ";
  else
    ss << "      ";
  if (over_threshold)
    ss << " othr ";
  else
    ss << "      ";
  //  ss << " time=" << (time >> 36) << ":" << (time & 0xFFFFFFFF);
  ss << " time=" << (time >> 52) << ":" << ((time >> 20) & 0xFFFFFFFF) << ":"
     << ((time >> 8) & 0xFFF) << ":" << (time & 0xFF);
  ss << " plane=" << plane_id << " strip=" << strip << " adc=" << adc;
  return ss.str();
}
