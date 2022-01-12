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
/// \brief Readout struct for CSPEC event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/DumpFile.h>

namespace Cspec {

struct __attribute__((packed)) Readout {
  /// \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "cspec_readouts"; }
  static uint16_t FormatVersion() { return 0; }

  /// \todo consider reordering these to optimize
  /// !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint32_t PulseTimeHigh{0};
  uint32_t PulseTimeLow{0};
  uint32_t PrevPulseTimeHigh{0};
  uint32_t PrevPulseTimeLow{0};
  uint32_t EventTimeHigh{0};
  uint32_t EventTimeLow{0};
  uint16_t DataSeqNum{0}; // Pulse Height in debug mode
  uint8_t OutputQueue{0};
  uint8_t RingId;
  uint8_t FENId;
  uint8_t TubeId{0};

  // !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  // \brief prints values for to_string purposes
  std::string debug() const;
};

} // namespace Cspec

namespace hdf5 {

namespace datatype {
template <> class TypeTrait<Loki::Readout> {
public:
  H5_COMPOUND_DEFINE_TYPE(Loki::Readout) {
    H5_COMPOUND_INIT;
    /// Make sure ALL member variables are inserted
    H5_COMPOUND_INSERT_MEMBER(PulseTimeHigh);
    H5_COMPOUND_INSERT_MEMBER(PulseTimeLow);
    H5_COMPOUND_INSERT_MEMBER(PrevPulseTimeHigh);
    H5_COMPOUND_INSERT_MEMBER(PrevPulseTimeLow);
    H5_COMPOUND_INSERT_MEMBER(EventTimeHigh);
    H5_COMPOUND_INSERT_MEMBER(EventTimeLow);
    H5_COMPOUND_INSERT_MEMBER(DataSeqNum);
    H5_COMPOUND_INSERT_MEMBER(OutputQueue);
    H5_COMPOUND_INSERT_MEMBER(RingId);
    H5_COMPOUND_INSERT_MEMBER(FENId);
    H5_COMPOUND_INSERT_MEMBER(TubeId);

    H5_COMPOUND_RETURN;
  }
};
} // namespace datatype

} // namespace hdf5

namespace Cspec {

using ReadoutFile = DumpFile<Readout>;

}
