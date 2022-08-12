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
///                              DĖMESIO
///
///                              UWAGA
///
///
///
///          MODIFY THIS FILE ONLY AFTER UNDERSTANDING THE FOLLOWING
///
///
///   Any changes to non-static variable definitions will likely break h5 file
/// writing and compatibility. If you rename, reorder or change the type of any
/// of the member variables, you MUST:
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

#include <common/DumpFile.h>

namespace Multigrid {

struct __attribute__((packed)) Readout {
  /// \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "mgmesytec_hits"; }
  static uint16_t FormatVersion() { return 0; }

  /// \todo consider reordering these to optimize
  /// !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
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

  // \brief prints values for to_string purposes
  std::string debug() const;
};

} // namespace Multigrid

namespace hdf5 {

namespace datatype {
template <> class TypeTrait<Multigrid::Readout> {
public:
  H5_COMPOUND_DEFINE_TYPE(Multigrid::Readout) {
    H5_COMPOUND_INIT;
    /// Make sure ALL member variables are inserted
    H5_COMPOUND_INSERT_MEMBER(trigger_count);
    H5_COMPOUND_INSERT_MEMBER(external_trigger);
    H5_COMPOUND_INSERT_MEMBER(module);
    H5_COMPOUND_INSERT_MEMBER(high_time);
    H5_COMPOUND_INSERT_MEMBER(low_time);
    H5_COMPOUND_INSERT_MEMBER(total_time);
    H5_COMPOUND_INSERT_MEMBER(bus);
    H5_COMPOUND_INSERT_MEMBER(channel);
    H5_COMPOUND_INSERT_MEMBER(adc);
    H5_COMPOUND_INSERT_MEMBER(time_diff);
    H5_COMPOUND_RETURN;
  }
};
} // namespace datatype

} // namespace hdf5

namespace Multigrid {

using ReadoutFile = DumpFile<Readout>;

}
