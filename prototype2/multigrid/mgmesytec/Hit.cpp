/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/Hit.h>
#include <sstream>

std::string MGHit::debug() const {
  std::stringstream ss;
  ss << " trigger_count=" << trigger_count;
  ss << " external_trigger=" << (external_trigger ? "true" : "false");
  ss << " module=" << static_cast<uint16_t>(module);
  ss << " high_time=" << high_time;
  ss << " low_time=" << low_time;
  ss << " total_time=" << total_time;
  ss << " bus=" << static_cast<uint16_t>(bus);
  ss << " channel=" << channel;
  ss << " adc=" << adc;
  ss << " time_diff=" << time_diff;
  return ss.str();
}
