#pragma once

#include <cinttypes>
#include <string>

struct Eventlet {
  uint64_t time{0};
  uint16_t plane_id{0};
  uint16_t strip{0};
  uint16_t adc{0};
  bool flag{false};
  bool over_threshold{false};

  std::string debug() const;
};
