// Copyright (C) 2019-2020 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief Jalousie Readout format - heed the warnings below
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
/// \brief Hit struct for Jalousie event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/DumpFile.h>

namespace Jalousie {

struct __attribute__ ((packed)) Readout {
  /// \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "jalousie_readouts"; }
  static uint16_t FormatVersion() { return 0; }

  /// \todo consider reordering these to optimize
  /// !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint32_t board{0};
  uint8_t sub_id{0};
  uint64_t time{0};
  uint8_t anode{0};
  uint8_t cathode{0};
  // !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  static constexpr uint32_t chopper_sub_id {std::numeric_limits<uint8_t>::max()};

  // \brief prints values for debug purposes
  std::string debug() const;
};

}

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Jalousie::Readout> {
public:
  H5_COMPOUND_DEFINE_TYPE(Jalousie::Readout) {
    H5_COMPOUND_INIT;
    /// Make sure ALL member variables are inserted
    H5_COMPOUND_INSERT_MEMBER(board);
    H5_COMPOUND_INSERT_MEMBER(sub_id);
    H5_COMPOUND_INSERT_MEMBER(time);
    H5_COMPOUND_INSERT_MEMBER(anode);
    H5_COMPOUND_INSERT_MEMBER(cathode);
    H5_COMPOUND_RETURN;
  }
};
}

}

namespace Jalousie {

using ReadoutFile = DumpFile<Readout>;

}
