// Copyright (C) 2021 European Spallation Source, see LICENSE file
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

struct __attribute__ ((packed)) ReadoutV2 {
  /// \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "jalousie_readouts"; }
  static uint16_t FormatVersion() { return 0; }

  /// \todo consider reordering these to optimize
  /// !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint32_t PulseTimeHigh {0};
  uint32_t PulseTimeLow {0};
  uint32_t EventTimeHigh {0};
  uint32_t EventTimeLow {0};
  uint8_t RingId;
  uint8_t FENId;
  uint8_t Module;
  uint8_t Sumo;
  uint8_t Strip;
  uint8_t Wire;
  uint8_t Segment;
  uint8_t Counter;
  // !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  static constexpr uint32_t chopper_sub_id {std::numeric_limits<uint8_t>::max()};

  // \brief prints values for debug purposes
  std::string debug() const;
};

}

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Jalousie::ReadoutV2> {
public:
  H5_COMPOUND_DEFINE_TYPE(Jalousie::ReadoutV2) {
    H5_COMPOUND_INIT;
    /// Make sure ALL member variables are inserted
    H5_COMPOUND_INSERT_MEMBER(PulseTimeHigh);
    H5_COMPOUND_INSERT_MEMBER(PulseTimeLow);
    H5_COMPOUND_INSERT_MEMBER(EventTimeHigh);
    H5_COMPOUND_INSERT_MEMBER(EventTimeLow);
    H5_COMPOUND_INSERT_MEMBER(RingId);
    H5_COMPOUND_INSERT_MEMBER(FENId);
    H5_COMPOUND_INSERT_MEMBER(Module);
    H5_COMPOUND_INSERT_MEMBER(Sumo);
    H5_COMPOUND_INSERT_MEMBER(Strip);
    H5_COMPOUND_INSERT_MEMBER(Wire);
    H5_COMPOUND_INSERT_MEMBER(Segment);
    H5_COMPOUND_INSERT_MEMBER(Counter);
    H5_COMPOUND_RETURN;
  }
};
}

}

namespace Jalousie {

using ReadoutFile = DumpFile<ReadoutV2>;

}
