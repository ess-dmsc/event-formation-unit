/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#pragma once

#include <multigrid/mgmesytec/Hit.h>
#include <common/DumpFile.h>

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Multigrid::Hit> {
public:
  using Type = Multigrid::Hit;
  using TypeClass = Compound;

  static TypeClass create(const Type & = Type()) {
    auto type = datatype::Compound::create(sizeof(Multigrid::Hit));
    type.insert("trigger_count",
                0,
                datatype::create<size_t>());
    type.insert("external_trigger",
                sizeof(size_t),
                datatype::create<std::int8_t>());
    type.insert("module",
                sizeof(size_t) + sizeof(std::int8_t),
                datatype::create<std::uint8_t>());
    type.insert("high_time",
                sizeof(size_t) + sizeof(std::int8_t) + sizeof(std::uint8_t),
                datatype::create<uint32_t>());
    type.insert("low_time",
                sizeof(size_t) + sizeof(std::int8_t) + sizeof(std::uint8_t) +
                    sizeof(std::uint32_t),
                datatype::create<uint32_t>());
    type.insert("total_time",
                sizeof(size_t) + sizeof(std::int8_t) + sizeof(std::uint8_t) +
                    2 * sizeof(std::uint32_t),
                datatype::create<uint64_t>());
    type.insert("bus",
                sizeof(size_t) + sizeof(std::int8_t) + sizeof(std::uint8_t) +
                    2 * sizeof(std::uint32_t) + sizeof(std::uint64_t),
                datatype::create<uint8_t>());
    type.insert("channel",
                sizeof(size_t) + sizeof(std::int8_t) + 2 * sizeof(std::uint8_t) +
                    2 * sizeof(std::uint32_t) + sizeof(std::uint64_t),
                datatype::create<uint16_t>());
    type.insert("adc",
                sizeof(size_t) + sizeof(std::int8_t) + 2 * sizeof(std::uint8_t) +
                    2 * sizeof(std::uint32_t) + sizeof(std::uint64_t) +
                    sizeof(std::uint16_t),
                datatype::create<uint16_t>());
    type.insert("time_diff",
                sizeof(size_t) + sizeof(std::int8_t) + 2 * sizeof(std::uint8_t) +
                    2 * sizeof(std::uint32_t) + sizeof(std::uint64_t) +
                    2 * sizeof(std::uint16_t),
                datatype::create<uint16_t>());
    return type;
  }
};
}

}

namespace Multigrid {

using HitFile = DumpFile<Hit>;

}
