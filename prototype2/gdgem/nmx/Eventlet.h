/** Copyright (C) 2016, 2017 European Spallation Source ERIC */

/** @file
 *
 *  @brief Eventlet struct for NMX event formation
 */

#pragma once

#include <cinttypes>
#include <limits>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <h5cpp/hdf5.hpp>
#pragma GCC diagnostic pop

struct Eventlet {
public:
  using strip_type = uint16_t;
  using adc_type = uint16_t;
  static constexpr strip_type strip_max_val{
      std::numeric_limits<strip_type>::max()};
  static constexpr adc_type adc_max_val{std::numeric_limits<adc_type>::max()};

public:
  Eventlet() {}

  double time{0};
  uint8_t plane_id{0};
  strip_type strip{0};
  adc_type adc{0};
  bool over_threshold{false};

  // @brief prints values for debug purposes
  std::string debug() const;
};


namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Eventlet>
{
public:
  using Type = Eventlet;
  using TypeClass = Compound;

  static TypeClass create(const Type & = Type())
  {
    auto type = datatype::Compound::create(sizeof(Eventlet));
    type.insert("time",
                0,
                datatype::create<double>());
    type.insert("plane_id",
                sizeof(double),
                datatype::create<std::uint8_t>());
    type.insert("strip",
                sizeof(double) + sizeof(std::uint8_t),
                datatype::create<Eventlet::strip_type>());
    type.insert("adc",
                sizeof(double) + sizeof(std::uint8_t) + sizeof(Eventlet::strip_type),
                datatype::create<Eventlet::adc_type>());
    type.insert("over_threshold",
                sizeof(double) + sizeof(std::uint8_t) + sizeof(Eventlet::strip_type)
                    + sizeof(Eventlet::adc_type),
                datatype::create<bool>());
    return type;
  }
};
}

}

