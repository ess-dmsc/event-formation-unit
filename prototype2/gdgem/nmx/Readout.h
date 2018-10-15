/* Copyright (C) 2017-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
///                              WARNING
///
///                              ACHTUNG
///
///                              AVISO
///
///                              ADVARSEL
///
///                              DEMESIO
///
///                              UWAGA
///
///
///
///          MODIFY THIS FILE ONLY AFTER READING THE FOLLOWING
///
///
///   Any changes to non-static variable definitions will likely break h5 file
/// writing and compatibility. If you rename, reorder or change the type of any
/// of the member variables, you must do the following:
///    A) Increment FormatVersion by 1
///    B) Ensure the hdf5 TypeTrait maps the struct correctly
///
/// If you cannot ensure the above, consult someone who can.
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <string>
#include <common/DumpFile.h>

// \todo initialize values to 0?
struct __attribute__ ((packed)) Readout
{
  // \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "srs_hits"; }
  static std::string FormatVersion() { return "1.0.0"; }

  // !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint8_t fec;
  uint8_t chip_id;
  uint32_t bonus_timestamp;
  uint32_t srs_timestamp;
  uint16_t channel;
  uint16_t bcid;
  uint16_t tdc;
  uint16_t adc;
  bool over_threshold;
  float ChipTimeNs {0.0};
  // !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  bool operator==(const Readout& other) const
  {
    return (
        (fec == other.fec) &&
        (chip_id == other.chip_id) &&
        (bonus_timestamp == other.bonus_timestamp) &&
        (srs_timestamp == other.srs_timestamp) &&
        (channel == other.channel) &&
        (bcid == other.bcid) &&
        (tdc == other.tdc) && (adc == other.adc) &&
        (over_threshold == other.over_threshold) &&
        (ChipTimeNs == other.ChipTimeNs)
    );
  }
};

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Readout>
{
private:
  static constexpr size_t u8 { sizeof(std::uint8_t)};
  static constexpr size_t u16 { sizeof(std::uint16_t)};
  static constexpr size_t u32 { sizeof(std::uint32_t)};
  static constexpr size_t b { sizeof(bool)};

public:
  using Type = Readout;
  using TypeClass = Compound;

  static TypeClass create(const Type & = Type())
  {
    auto type = datatype::Compound::create(sizeof(Readout));
    type.insert("fec",
                0,
                datatype::create<std::uint8_t>());
    type.insert("chip_id",
                u8,
                datatype::create<std::uint8_t>());
    type.insert("bonus_timestamp",
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
    type.insert("ChipTimeNs",
                2*u8 + 2*u32 + 4*u16 + b,
                datatype::create<float>());
    return type;
  }
};
}

}

using ReadoutFile = DumpFile<Readout>;
