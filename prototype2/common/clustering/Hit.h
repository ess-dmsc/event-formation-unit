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
/// \brief Hit struct for general clustering solution
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/DumpFile.h>

struct __attribute__ ((packed)) Hit {
  /// \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "efu_hits"; }
  static uint16_t FormatVersion() { return 0; }

  /// \todo consider reordering these to optimize
  /// !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint64_t time{0};
  uint8_t plane{0};
  uint16_t coordinate{0};
  uint16_t weight{0};
  // !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  // \brief prints values for debug purposes
  std::string debug() const;
};

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Hit> {
public:
  H5_COMPOUND_DEFINE_TYPE(Hit) {
    H5_COMPOUND_INIT;
    /// Make sure ALL member variables are inserted
    H5_COMPOUND_INSERT_MEMBER(time);
    H5_COMPOUND_INSERT_MEMBER(plane);
    H5_COMPOUND_INSERT_MEMBER(coordinate);
    H5_COMPOUND_INSERT_MEMBER(weight);
    H5_COMPOUND_RETURN;
  }
};
}

}

using HitFile = DumpFile<Hit>;
