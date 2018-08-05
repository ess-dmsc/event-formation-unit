/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

#include <multigrid/mgmesytec/Hit.h>
#include <sstream>

std::string MGHit::debug() const {
  std::stringstream ss;
  ss << " trigger_count=" << trigger_count;
  ss << " total_time=" << total_time;
  ss << " external_trigger=" << (external_trigger ? "true" : "false");

  if (!external_trigger) {
    ss << " bus=" << static_cast<uint16_t>(bus);
    ss << " channel=" << channel;
    ss << " adc=" << adc;
  }

  // \todo don't use this
  ss << " high_time=" << high_time;
  ss << " low_time=" << low_time;
  ss << " time_diff=" << time_diff;

  // \todo use this
  //ss << " module=" << static_cast<uint16_t>(module);

  return ss.str();
}
