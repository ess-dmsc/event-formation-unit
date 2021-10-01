// Copyright (C) 2016-2020 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implementation of file saving functionality
///
//===----------------------------------------------------------------------===//

#include <cassert>
#include <common/DataSave.h>
#include <common/time/TimeString.h>
#include <common/debug/Log.h>
#include <fmt/format.h>

//#undef TRC_LEVEL
//#define TRC_LEVEL TRC_L_DEB


//
// DataSave::DataSave(std::string filename) : filename_prefix(filename) {
//
//   if ((fd = open(filename_prefix.c_str(), flags, mode)) < 0) {
//     std::string msg = "DataSave: open(" + filename + ") failed";
//     perror(msg.c_str());
//   }
// }

DataSave::DataSave(std::string name, uint64_t maxlen)
    : filename_prefix(name), maxfilesize(maxlen) {

  startTime = timeString();
  createfile();
}

int DataSave::tofile(char *buffer, uint64_t len) {
  if (fd < 0)
    return -1;

  return adjustfilesize(write(fd, buffer, len));
}

int DataSave::tofile(std::string text) {
  return tofile(text.c_str(), text.size());
}

int DataSave::tofile(const char *fmt, ...) {
  if (fd < 0)
    return -1;

  int retlen = 0;
  va_list args;
  va_start(args, fmt);
  /** \brief prevent buffer overrun, but data could be truncated */
  auto maxwritelen = BUFFERSIZE + MARGIN - bufferlen;
  int ret = vsnprintf(buffer + bufferlen, maxwritelen, fmt, args);
  va_end(args);

  if (ret < 0) {
    LOG(UTILS, Sev::Error, "vsnprintf failed");
    return ret;
  }

  if (ret > maxwritelen) {
    LOG(UTILS, Sev::Warning, "datasave has been truncated");
    ret = maxwritelen;
  }

  bufferlen += ret;
  if (bufferlen >= BUFFERSIZE) {
    LOG(UTILS, Sev::Debug, "Writing chunk of size {}", bufferlen);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    write(fd, buffer, bufferlen);
#pragma GCC diagnostic pop
    retlen = bufferlen;
    bufferlen = 0;
  }
  return adjustfilesize(retlen);
}

DataSave::~DataSave() {
  LOG(UTILS, Sev::Debug, "~DataSave bufferlen {}", bufferlen);
  if (bufferlen > 0) {
    LOG(UTILS, Sev::Info, "Flushing DataSave buffer");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    write(fd, buffer, bufferlen);
#pragma GCC diagnostic pop
  }
  close(fd);
}

/** Private functions below, API functions above */

std::string DataSave::getfilename() {
  return filename_prefix + startTime + "_" +
         std::to_string(sequence_number - 1) + ".csv";
}

void DataSave::createfile() {
  curfilesize = 0;

  close(fd);

  std::string fileName = filename_prefix + startTime + "_" +
                         std::to_string(sequence_number) + ".csv";

  if ((fd = open(fileName.c_str(), flags, mode)) < 0) {
    throw std::runtime_error(fmt::format("DataSave: open({}) failed", fileName));
  }

  sequence_number++;
}

// Helper function
int DataSave::adjustfilesize(int returnval) {
  if (returnval > 0) {
    curfilesize += returnval;
  }
  if (curfilesize >= maxfilesize) {
    createfile();
  }
  return returnval;
}
