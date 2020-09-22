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
/// \brief Readout struct for Multiblade CAEN event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#define H5_USE_NEW_COMPOUND_IMPL 0
#include <common/DumpFile.h>
#undef H5_USE_NEW_COMPOUND_IMPL

namespace Multiblade {

struct __attribute__ ((packed)) Readout {
  static const char *DatasetName() { return "mbcaen_readouts"; }
  static uint16_t FormatVersion() { return 0; }

  /// \todo consider reordering these to optimize
  /// !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint64_t global_time {0};
  uint32_t digitizer {0};
  uint32_t local_time {0};
  uint16_t channel {0};
  uint16_t adc {0};
  // !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  // \brief prints values for to_string purposes
  std::string debug() const;
};

}

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Multiblade::Readout> {
public:
  H5_COMPOUND_DEFINE_TYPE(Multiblade::Readout) {
    H5_COMPOUND_INIT;
    /// Make sure ALL member variables are inserted
    H5_COMPOUND_INSERT_MEMBER(global_time);
    H5_COMPOUND_INSERT_MEMBER(digitizer);
    H5_COMPOUND_INSERT_MEMBER(local_time);
    H5_COMPOUND_INSERT_MEMBER(channel);
    H5_COMPOUND_INSERT_MEMBER(adc);
    H5_COMPOUND_RETURN;
  }
};
}

}

namespace Multiblade {

using ReadoutFile = DumpFile<Readout>;

}
