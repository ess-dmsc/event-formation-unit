/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <string>
#include <common/DumpFile.h>

struct OldReadout
{
  uint8_t fec;
  uint8_t chip_id;
  uint32_t bonus_timestamp;
  uint32_t srs_timestamp;
  uint16_t channel;
  uint16_t bcid;
  uint16_t tdc;
  uint16_t adc;
  bool over_threshold;

  static const char *DatasetName() { return "srs_hits"; }
  static std::string FormatVersion() { return "1.0.0"; }
};


namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<OldReadout>
{
private:
  static constexpr size_t u8 { sizeof(std::uint8_t)};
  static constexpr size_t u16 { sizeof(std::uint16_t)};
  static constexpr size_t u32 { sizeof(std::uint32_t)};
  static constexpr size_t b { sizeof(bool)};

public:
  using Type = OldReadout;
  using TypeClass = Compound;

  static TypeClass create(const Type & = Type())
  {
    auto type = datatype::Compound::create(sizeof(OldReadout));
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
    return type;
  }
};
}

}

using OldReadoutFile = DumpFile<OldReadout>;
