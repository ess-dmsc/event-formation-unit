// Copyright (C) 2021 - 2023 European Spallation Source, see LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
/// \brief DREAM Readout format - heed the warnings below
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
/// \brief Readout struct for Dream event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/DumpFile.h>

namespace Dream {

struct __attribute__((packed)) Readout {
  /// \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "dream_readouts"; }
  static uint16_t FormatVersion() { return 1; }

  /// \todo consider reordering these to optimize
  /// !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint32_t PulseTimeHigh{0};
  uint32_t PulseTimeLow{0};
  uint32_t EventTimeHigh{0};
  uint32_t EventTimeLow{0};
  uint8_t FiberId{0};
  uint8_t FENId{0};
  uint8_t OperatingMode{0};
  uint8_t UnitId{0};
  uint8_t Cathode{0};
  uint8_t Anode{0};
  // !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  static constexpr uint32_t chopper_sub_id{std::numeric_limits<uint8_t>::max()};

  /// \brief prints values for debug purposes
  std::string debug() const;
};

} // namespace Dream

namespace hdf5 {

namespace datatype {
template <> class TypeTrait<Dream::Readout> {
public:
  H5_COMPOUND_DEFINE_TYPE(Dream::Readout) {
    H5_COMPOUND_INIT;
    /// Make sure ALL member variables are inserted
    H5_COMPOUND_INSERT_MEMBER(PulseTimeHigh);
    H5_COMPOUND_INSERT_MEMBER(PulseTimeLow);
    H5_COMPOUND_INSERT_MEMBER(EventTimeHigh);
    H5_COMPOUND_INSERT_MEMBER(EventTimeLow);
    H5_COMPOUND_INSERT_MEMBER(FiberId);
    H5_COMPOUND_INSERT_MEMBER(FENId);
    H5_COMPOUND_INSERT_MEMBER(OperatingMode);
    H5_COMPOUND_INSERT_MEMBER(UnitId);
    H5_COMPOUND_INSERT_MEMBER(Cathode);
    H5_COMPOUND_INSERT_MEMBER(Anode);
    H5_COMPOUND_RETURN;
  }
};
} // namespace datatype

} // namespace hdf5

namespace Dream {

using ReadoutFile = DumpFile<Readout>;

}
