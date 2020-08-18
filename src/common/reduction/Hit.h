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
/// \file Hit.h
///
/// \brief Hit struct for general clustering solution
///
//===----------------------------------------------------------------------===//

#pragma once

#define H5_USE_NEW_COMPOUND_IMPL 1
#include <common/DumpFile.h>
#undef H5_USE_NEW_COMPOUND_IMPL
#include <limits>

struct __attribute__ ((packed)) Hit {
  static const char *DatasetName() { return "efu_hits"; }
  static uint16_t FormatVersion() { return 0; }

  /// !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint64_t time{0};
  uint16_t coordinate{0};
  uint16_t weight{0};
  // \todo uint8 might not be enough, if detectors have more independent modules/segments
  uint8_t plane{0};
  /// !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  /// \brief prints values for debug purposes
  std::string to_string() const;

  static constexpr uint16_t InvalidCoord {std::numeric_limits<uint16_t>::max()};
  static constexpr uint8_t InvalidPlane {std::numeric_limits<uint8_t>::max()};
  static constexpr uint8_t PulsePlane {std::numeric_limits<uint8_t>::max() - 1};
};

#define H5_USE_NEW_COMPOUND_IMPL 1

#if H5_USE_NEW_COMPOUND_IMPL == 0 /////// TODO put our data in same namespace and class?
namespace hdf5 {
namespace datatype {
template<>
class TypeTrait<Hit> {
public:
#endif
  H5_COMPOUND_DEFINE_TYPE(Hit) //{
    H5_COMPOUND_INIT
    /// Make sure ALL member variables are inserted
    H5_COMPOUND_INSERT_MEMBER(time)
    H5_COMPOUND_INSERT_MEMBER(coordinate)
    H5_COMPOUND_INSERT_MEMBER(weight)
    H5_COMPOUND_INSERT_MEMBER(plane)
    H5_COMPOUND_RETURN
#if H5_USE_NEW_COMPOUND_IMPL == 0
  }
};
}
}
#endif

#if H5_USE_NEW_COMPOUND_IMPL == 0
using HitFile = DumpFile<Hit>;
#else
using HitFile = PrimDumpFile<Hit>;
#endif

#undef H5_USE_NEW_COMPOUND_IMPL
