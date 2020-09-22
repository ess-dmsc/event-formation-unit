/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <jalousie/generators/CdtFile.h>
#include <fmt/format.h>

namespace Jalousie {

const uint64_t kTypeMask{uint64_t(0x3) << 62};
const uint64_t kNeutronDataType{uint64_t(0x0) << 62};
const uint64_t kMetaDataType{uint64_t(0x2) << 62};
const uint64_t kAdcDataType{uint64_t(0x3) << 62};

// const uint64_t kMetaDataIndex0{uint64_t(0x0) << 58};
// const uint64_t kMetaDataIndex1{uint64_t(0x1) << 58};
// const uint64_t kMetaDataIndex2{uint64_t(0x2) << 58};
// const uint64_t kMetaDataIndex3{uint64_t(0x3) << 58};
// const uint64_t kMetaDataIndex4{uint64_t(0x4) << 58};
// const uint64_t kMetaDataIndex5{uint64_t(0x5) << 58};
// const uint64_t kMetaDataIndex6{uint64_t(0x6) << 58};
// const uint64_t kMetaDataIndex7{uint64_t(0x7) << 58};
// const uint64_t kMetaDataIndex8{uint64_t(0x8) << 58};

const uint64_t kMetaDataMask{uint64_t(0xF) << 54};
const uint64_t kMetaDataSubIndex0{uint64_t(0x0) << 54};
const uint64_t kMetaDataSubIndex1{uint64_t(0x1) << 54};
// const uint64_t kMetaDataSubIndex2{uint64_t(0x2) << 54};
// const uint64_t kMetaDataSubIndex3{uint64_t(0x3) << 54};
// const uint64_t kMetaDataSubIndex4{uint64_t(0x4) << 54};
// const uint64_t kMetaDataSubIndex5{uint64_t(0x5) << 54};
// const uint64_t kMetaDataSubIndex6{uint64_t(0x6) << 54};
// const uint64_t kMetaDataSubIndex7{uint64_t(0x7) << 54};
// const uint64_t kMetaDataSubIndex8{uint64_t(0x8) << 54};
// const uint64_t kMetaDataSubIndex9{uint64_t(0x9) << 54};
// const uint64_t kMetaDataSubIndex10{uint64_t(0xA) << 54};

CdtFile::CdtFile(const std::string &FilePath) {
  Data.reserve(ChunkSize);

  file_.open(FilePath, std::ios::binary);
  if (!file_.is_open()) {
    throw std::runtime_error(
        fmt::format("<CdtFile> {} could not be opened!\n", FilePath));
  }

  int header[8];
  file_.read((char *)header, 8*sizeof(int));

  // survey file
  while (file_) {
    read_one();
  }
  file_.close();
  file_.open(FilePath, std::ios::binary);
  file_.read((char *)header, 8*sizeof(int));
  have_board = false;
  readouts_expected = survey_results.chopper_pulses + survey_results.events_found;
}

size_t CdtFile::total() const {
  return readouts_expected;
}

size_t CdtFile::read() {
  Data.clear();
  while (file_ && (Data.size() < ChunkSize)) {
    if (read_one())
      Data.push_back(readout);
  }
  return Data.size();
}

size_t CdtFile::read(char *buffer) {
  auto size = read();
  memcpy(buffer, Data.data(), sizeof(Readout) * size);
  return sizeof(Readout) * size;
}

bool CdtFile::read_one() {
  file_.read((char *) &data, sizeof(uint64_t));

  switch (data & kTypeMask) {
    // NEUTRON
  case kNeutronDataType: {
    if (!have_board) {
      survey_results.unidentified_board++;
      return false;
    }
    survey_results.events_found++;
    readout.anode = data & 0x3F;
    readout.cathode = (data & 0x1FC0) >> 6;
    readout.time = (data & (0x1FFFFFFFFFFFE000ULL)) >> 13;
    readout.sub_id = (data & ((1ULL) << 58));
    return true;
  }
    break;
  case kMetaDataType: {
    survey_results.metadata_found++;

    switch (data & kMetaDataMask) {
    case kMetaDataSubIndex0: {
      // chopper timestamp
      if (!have_board) {
        survey_results.unidentified_board++;
        return false;
      }
      readout.sub_id = Readout::chopper_sub_id;
      readout.time = (data & (uint64_t(0xFFFFFFFFFFFF)));
      survey_results.chopper_pulses++;
      return true;
    }
      break;
    case kMetaDataSubIndex1: {
      // board ID
      have_board = true;
      readout.board = (data & 0xffffff);
      survey_results.board_blocks++;
    }
      break;
    default:break;
    }
    break;
  }
    break;
  case kAdcDataType: {
    survey_results.adc_found++;
  }
    break;
  default:break;
  }

  return false;
}

}
