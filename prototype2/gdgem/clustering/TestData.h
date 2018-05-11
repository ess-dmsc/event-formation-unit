#pragma once

#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <h5cpp/hdf5.hpp>
#pragma GCC diagnostic pop

struct Hit
{
  uint8_t fec;
  uint8_t chip_id;
  uint32_t frame_counter;
  uint32_t srs_timestamp;
  uint16_t channel;
  uint16_t bcid;
  uint16_t tdc;
  uint16_t adc;
  bool over_threshold;

  bool operator==(const Hit& other) const
  {
    return ((fec == other.fec) &&
        (chip_id == other.chip_id) &&
        (frame_counter == other.frame_counter) &&
        (srs_timestamp == other.srs_timestamp) &&
        (channel == other.channel) &&
        (bcid == other.bcid) &&
        (tdc == other.tdc) && (adc == other.adc) &&
        (over_threshold == other.over_threshold));
  }
};

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Hit>
{
private:
  static constexpr size_t u8 { sizeof(std::uint8_t)};
  static constexpr size_t u16 { sizeof(std::uint16_t)};
  static constexpr size_t u32 { sizeof(std::uint32_t)};
  static constexpr size_t b { sizeof(bool)};

public:
  using Type = Hit;
  using TypeClass = Compound;

  static TypeClass create(const Type & = Type())
  {
    auto type = datatype::Compound::create(sizeof(Hit));
    type.insert("fec",
                0,
                datatype::create<std::uint8_t>());
    type.insert("chip_id",
                u8,
                datatype::create<std::uint8_t>());
    type.insert("frame_counter",
                2*u8,
                datatype::create<std::uint32_t>());
    type.insert("srs_timestamp",
                2*u8 + u32,
                datatype::create<std::uint32_t>());
    type.insert("channel",
                2*u8 + 2*u32,
                datatype::create<std::uint16_t>());
    type.insert("bcid",
                2*u8 + 2*u32 + u16,
                datatype::create<std::uint16_t>());
    type.insert("tdc",
                2*u8 + 2*u32 + 2*u16,
                datatype::create<std::uint16_t>());
    type.insert("adc",
                2*u8 + 2*u32 + 3*u16,
                datatype::create<std::uint16_t>());
    type.insert("over_threshold",
                2*u8 + 2*u32 + 4*u16,
                datatype::create<bool>());
    return type;
  }
};
}

}

struct SRSHitIO
{
  std::vector<Hit> data;

  void read(std::string file);
  void write(std::string file);
};