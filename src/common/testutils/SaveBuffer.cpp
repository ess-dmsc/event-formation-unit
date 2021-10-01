// Copyright (C) 2018-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Save a buffer to file - only to be used in unit tests!
//===----------------------------------------------------------------------===//

#include <common/testutils/SaveBuffer.h>
#include <fcntl.h>
#include <fmt/format.h>
#include <sys/types.h>
#include <unistd.h>

void saveBuffer(std::string filename, void *buffer, uint64_t datasize) {
  int fd;
  const int flags = O_TRUNC | O_CREAT | O_RDWR;
  const int mode = S_IRUSR | S_IWUSR;

  if ((fd = open(filename.c_str(), flags, mode)) < 0) {
    throw std::runtime_error(fmt::format("DataSave: open({}) failed", filename));
  }

  if (::write(fd, buffer, datasize) < 0) {
    throw std::runtime_error(fmt::format("DataSave: write({}) failed", filename));
  }
}

void deleteFile(std::string filename) {
  remove(filename.c_str());
}
