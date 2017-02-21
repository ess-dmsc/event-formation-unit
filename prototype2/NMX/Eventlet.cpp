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

void Eventlet::write_packet(std::vector<uint32_t> &packet) const {
  if (packet.size() != 4)
    packet.resize(4, 0);
  packet[0] = time >> 32;
  packet[1] = time & 0xFFFFFFFF;
  packet[2] = (uint32_t(plane_id) << 16) | strip;
  packet[3] = (uint32_t(flag) << 16) | (uint32_t(over_threshold) << 17) | adc;
}

void Eventlet::read_packet(const std::vector<uint32_t> &packet) {
  if (packet.size() == 4) {
    time = (uint64_t(packet.at(0)) << 32) | uint64_t(packet.at(1));
    plane_id = packet.at(2) >> 16;
    strip = packet.at(2) & 0xFFFF;
    flag = (packet.at(3) >> 16) & 0x1;
    over_threshold = (packet.at(3) >> 17) & 0x1;
    adc = packet.at(3) & 0xFFFF;
  }
}

std::vector<uint32_t> Eventlet::to_packet() const {
  std::vector<uint32_t> ret(4);
  write_packet(ret);
  return ret;
}

Eventlet Eventlet::from_packet(const std::vector<uint32_t> &packet) {
  Eventlet ret;
  ret.read_packet(packet);
  return ret;
}
