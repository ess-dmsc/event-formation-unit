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
/// \brief Readout struct for NMX event formation
///
//===----------------------------------------------------------------------===//

#pragma once

#include <common/DumpFile.h>

struct __attribute__ ((packed)) Readout {
  // \todo use constexpr string_view when c++17 arrives
  static std::string DatasetName() { return "srs_hits"; }
  static uint16_t FormatVersion() { return 0; }

  // !!! DO NOT MODIFY BELOW - READ HEADER FIRST !!!
  uint8_t fec{0};
  uint8_t chip_id{0};
  uint32_t bonus_timestamp{0};
  uint32_t srs_timestamp{0};
  uint16_t channel{0};
  uint16_t bcid{0};
  uint16_t tdc{0};
  uint16_t adc{0};
  bool over_threshold{false};
  float ChipTimeNs{0.0};
  // !!! DO NOT MODIFY ABOVE -- READ HEADER FIRST !!!

  bool operator==(const Readout &other) const {
    return (
        (fec == other.fec) &&
            (chip_id == other.chip_id) &&
            (bonus_timestamp == other.bonus_timestamp) &&
            (srs_timestamp == other.srs_timestamp) &&
            (channel == other.channel) &&
            (bcid == other.bcid) &&
            (tdc == other.tdc) && (adc == other.adc) &&
            (over_threshold == other.over_threshold) &&
            (ChipTimeNs == other.ChipTimeNs)
    );
  }
};

namespace hdf5 {

namespace datatype {
template<>
class TypeTrait<Readout> {
public:
  H5_COMPOUND_DEFINE_TYPE(Readout) {
    H5_COMPOUND_INIT;
    H5_COMPOUND_INSERT_MEMBER(fec);
    H5_COMPOUND_INSERT_MEMBER(chip_id);
    H5_COMPOUND_INSERT_MEMBER(bonus_timestamp);
    H5_COMPOUND_INSERT_MEMBER(srs_timestamp);
    H5_COMPOUND_INSERT_MEMBER(channel);
    H5_COMPOUND_INSERT_MEMBER(bcid);
    H5_COMPOUND_INSERT_MEMBER(tdc);
    H5_COMPOUND_INSERT_MEMBER(adc);
    H5_COMPOUND_INSERT_MEMBER(over_threshold);
    H5_COMPOUND_INSERT_MEMBER(ChipTimeNs);
    H5_COMPOUND_RETURN;
  }
};
}

}

using ReadoutFile = DumpFile<Readout>;
