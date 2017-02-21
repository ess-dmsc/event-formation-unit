#pragma once

#include <cinttypes>
#include <string>
#include <vector>

struct Eventlet {
  uint64_t time{0};
  uint16_t plane_id{0};
  uint16_t strip{0};
  uint16_t adc{0};
  bool flag{false};
  bool over_threshold{false};

  std::string debug() const;

  void write_packet(std::vector<uint32_t> &packet) const;
  void read_packet(const std::vector<uint32_t> &packet);

  std::vector<uint32_t> to_packet() const;
  static Eventlet from_packet(const std::vector<uint32_t> &packet);
};
