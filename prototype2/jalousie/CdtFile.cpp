/// Copyright (C) 2016-2018 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
///
//===----------------------------------------------------------------------===//

#include <jalousie/CdtFile.h>
#include <fmt/format.h>

namespace Jalousie {

CdtFile::CdtFile(const boost::filesystem::path &FilePath) {
  Data.reserve(ChunkSize);

  file_.open(FilePath.string(), std::ios::binary);
  if (!file_.is_open())
  {
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

  while (file_)
  {
    int32_t data = 0;
    file_.read((char*)&data, sizeof(int32_t));

    if (data == 0)
    {
      MaxSize++;
//      event_locations_.push_back(address - addr_begin);
      address = file_.tellg();
    }
  }
}

}
