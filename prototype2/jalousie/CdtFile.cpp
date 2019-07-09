/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <jalousie/CdtFile.h>
#include <fmt/format.h>

namespace Jalousie {

const uint64_t kNeutronDataType{0x0ULL << 62};
const uint64_t kMetaDataType{0x2ULL << 62};
const uint64_t kAdcDataType{0x3ULL << 62};

const uint64_t kMetaDataIndex0{0x0ULL << 58};
const uint64_t kMetaDataIndex1{0x1ULL << 58};
const uint64_t kMetaDataIndex2{0x2ULL << 58};
const uint64_t kMetaDataIndex3{0x3ULL << 58};
const uint64_t kMetaDataIndex4{0x4ULL << 58};
const uint64_t kMetaDataIndex5{0x5ULL << 58};
const uint64_t kMetaDataIndex6{0x6ULL << 58};
const uint64_t kMetaDataIndex7{0x7ULL << 58};
const uint64_t kMetaDataIndex8{0x8ULL << 58};

const uint64_t kMetaDataSubIndex0{0x0ULL << 54};
const uint64_t kMetaDataSubIndex1{0x1ULL << 54};
const uint64_t kMetaDataSubIndex2{0x2ULL << 54};
const uint64_t kMetaDataSubIndex3{0x3ULL << 54};
const uint64_t kMetaDataSubIndex4{0x4ULL << 54};
const uint64_t kMetaDataSubIndex5{0x5ULL << 54};
const uint64_t kMetaDataSubIndex6{0x6ULL << 54};
const uint64_t kMetaDataSubIndex7{0x7ULL << 54};
const uint64_t kMetaDataSubIndex8{0x8ULL << 54};
const uint64_t kMetaDataSubIndex9{0x9ULL << 54};
const uint64_t kMetaDataSubIndex10{0xAULL << 54};

CdtFile::CdtFile(const boost::filesystem::path &FilePath) {
  Data.reserve(ChunkSize);

  file_.open(FilePath.string(), std::ios::binary);
  if (!file_.is_open()) {
    throw std::runtime_error(
        fmt::format("<CdtFile> {} could not be opened!\n", FilePath.string()));
  }

  surveyFile();

  file_.close();
  file_.open(FilePath.string(), std::ios::binary);
}

//size_t CdtFile::count() const {
//  return 0;
//}

//void CdtFile::readAt(size_t Index, size_t Count) {
//  Data.resize(Count);
//}

size_t CdtFile::read() {
  return 0;
}

void CdtFile::surveyFile() {
  MaxSize = 0;
//  auto addr_begin = file_.tellg();
  auto address = file_.tellg();

  while (file_) {
    uint64_t data = 0;
    file_.read((char *) &data, sizeof(uint64_t));

    switch (data & ((0x3ULL) << 62)) {
      // NEUTRON
    case kNeutronDataType: {
      stats.events_found++;
    }
      break;
    case kMetaDataType: {
      stats.pulses_found++;
    }
      break;
    case kAdcDataType:break;
    default:break;
    }

    if (data == 0) {
      MaxSize++;
//      event_locations_.push_back(address - addr_begin);
      address = file_.tellg();
    }
  }
}

}
