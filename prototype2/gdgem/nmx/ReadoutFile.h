/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <gdgem/nmx/Readout.h>
#include <common/DumpFile.h>


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
    return type;
  }
};
}

}

using ReadoutFile = DumpFile<Readout>;
