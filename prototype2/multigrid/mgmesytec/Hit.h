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
/// \file
///
/// \brief Hit struct for Multigrid event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <string>
#include <common/DumpFile.h>

namespace Multigrid {

struct __attribute__ ((packed)) Hit {
  // \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "mgmesytec_hits"; }
  static std::string FormatVersion() { return "1.0.0"; }

  // !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  size_t trigger_count{0};
  int8_t external_trigger{0};
  uint8_t module{0};
  uint32_t high_time{0};
  uint32_t low_time{0};
  uint64_t total_time{0};
  uint8_t bus{0};
  uint16_t channel{0};
  uint16_t adc{0};
  uint16_t time_diff{0};
  // !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  // \brief prints values for debug purposes
  std::string debug() const;
};

}

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
