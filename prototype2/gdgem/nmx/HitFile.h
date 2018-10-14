/* Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file */
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <gdgem/nmx/Hit.h>
#include <common/DumpFile.h>

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Hit>
{
public:
  using Type = Hit;
  using TypeClass = Compound;

  static TypeClass create(const Type & = Type())
  {
    auto type = datatype::Compound::create(sizeof(Hit));
    type.insert("time",
                0,
                datatype::create<double>());
    type.insert("plane_id",
                sizeof(double),
                datatype::create<std::uint8_t>());
    type.insert("strip",
                sizeof(double) + sizeof(std::uint8_t),
                datatype::create<Hit::strip_type>());
    type.insert("adc",
                sizeof(double) + sizeof(std::uint8_t) + sizeof(Hit::strip_type),
                datatype::create<Hit::adc_type>());
    return type;
  }
};
}

}

using HitFile = DumpFile<Hit>;
