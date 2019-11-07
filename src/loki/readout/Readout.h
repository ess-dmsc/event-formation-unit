/* Copyright (C) 2019 European Spallation Source, ERIC. See LICENSE file */
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
/// \brief Readout struct for LOKI event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/DumpFile.h>

namespace Loki {

struct __attribute__ ((packed)) Readout {
  /// \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "loki_readouts"; }
  static uint16_t FormatVersion() { return 0; }

  /// \todo consider reordering these to optimize
  /// !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint64_t global_time {0};
  uint32_t local_time {0};
  uint16_t amp_a{0};
  uint16_t amp_b{0};
  uint16_t amp_c{0};
  uint16_t amp_d{0};
  uint8_t fpga_id {0};
  uint8_t tube_id {0};
  // !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  // \brief prints values for to_string purposes
  std::string debug() const;
};

}

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Loki::Readout> {
public:
  H5_COMPOUND_DEFINE_TYPE(Loki::Readout) {
    H5_COMPOUND_INIT;
    /// Make sure ALL member variables are inserted
    H5_COMPOUND_INSERT_MEMBER(global_time);
    H5_COMPOUND_INSERT_MEMBER(local_time);
    H5_COMPOUND_INSERT_MEMBER(amp_a);
    H5_COMPOUND_INSERT_MEMBER(amp_b);
    H5_COMPOUND_INSERT_MEMBER(amp_c);
    H5_COMPOUND_INSERT_MEMBER(amp_d);
    H5_COMPOUND_INSERT_MEMBER(fpga_id);
    H5_COMPOUND_INSERT_MEMBER(tube_id);
    H5_COMPOUND_RETURN;
  }
};
}

}

namespace Loki {

using ReadoutFile = DumpFile<Readout>;

}
