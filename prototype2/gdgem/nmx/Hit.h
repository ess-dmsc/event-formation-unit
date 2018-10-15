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
/// \brief Hit struct for NMX event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <cinttypes>
#include <limits>
#include <string>
#include <common/DumpFile.h>

struct __attribute__ ((packed)) Hit {
  // \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "gdgem_hits"; }
  static std::string FormatVersion() { return "1.0.0"; }

  // !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  using strip_type = uint16_t;
  using adc_type = uint16_t;
  double time{0};
  uint8_t plane_id{0};
  strip_type strip{0};
  adc_type adc{0};
  // !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  // \brief prints values for debug purposes
  std::string debug() const;

  static constexpr strip_type strip_max_val{std::numeric_limits<strip_type>::max()};
  static constexpr adc_type adc_max_val{std::numeric_limits<adc_type>::max()};
};

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Hit> {
public:
  H5_COMPOUND_DEFINE_TYPE(Hit) {
    H5_COMPOUND_INIT;
    H5_COMPOUND_INSERT_MEMBER(time);
    H5_COMPOUND_INSERT_MEMBER(plane_id);
    H5_COMPOUND_INSERT_MEMBER(strip);
    H5_COMPOUND_INSERT_MEMBER(adc);
    H5_COMPOUND_RETURN;
  }
};
}

}

using HitFile = DumpFile<Hit>;
